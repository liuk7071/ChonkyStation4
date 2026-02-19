// Adapted from ChonkyStation3

#include "Filesystem.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>
#include <OS/SceObj.hpp>


namespace PS4::FS {

MAKE_LOG_FUNCTION(log, filesystem);

std::unordered_map<Device, fs::path> mounted_devices;
std::unordered_map<u64, std::unique_ptr<File>> open_files;
std::unordered_map<u64, Directory> open_dirs;

void mount(Device device, fs::path path) {
    mounted_devices[device] = path;
    log("Mounted device %s at %s\n", deviceToString(device).c_str(), path.generic_string().c_str());
}

void umount(Device device) {
    if (!mounted_devices.contains(device)) Helpers::panic("Tried to unmount an unmounted device (%s)\n", deviceToString(device).c_str());

    mounted_devices.erase(device);
    log("Unmounted device %s\n", deviceToString(device).c_str());
}

void init() {
    // Create mount point directories if they don't exist
    for (auto& i : mounted_devices)
        fs::create_directories(i.second);
}

static int fileno = 1;

u64 open(fs::path path, u32& err, u32 flags) {
    const fs::path host_path = guestPathToHost(path);
    std::string mode = "";
    if      ((flags & 3) == SCE_KERNEL_O_RDONLY) mode = "rb";
    else if ((flags & 3) == SCE_KERNEL_O_WRONLY) mode = "wb";
    else if ((flags & 3) == SCE_KERNEL_O_RDWR)   mode = "rb+";
    Helpers::debugAssert(!mode.empty(), "Filesystem: invalid open mode flags 0x%x\n", flags);
    
    
    const bool append = flags & SCE_KERNEL_O_APPEND;
    const bool create = flags & SCE_KERNEL_O_CREAT;
    const bool trunc  = flags & SCE_KERNEL_O_TRUNC;
    const bool excl   = flags & SCE_KERNEL_O_EXCL;
    const bool dir    = flags & SCE_KERNEL_O_DIRECTORY;

    if (append) {
        Helpers::panic("Filesystem: append (TODO)\n");
    }

    if (excl) {
        Helpers::panic("Filesystem: excl (TODO)\n");
    }

    // If it's not a directory and the dir flag was specified, return an error
    const bool is_dir = fs::is_directory(host_path);
    if (dir && !is_dir) {
        err = POSIX_ENOTDIR;
        return 0;
    }

    if (!fs::exists(host_path)) {
        // For SCE_KERNEL_O_CREAT, the parent path must exist, otherwise return an error regardless (TODO: This was PS3 behavior, verify on PS4)
        if (!create || !fs::exists(host_path.parent_path())) {
            log("WARNING: Tried to open non-existing file %s\n", path.generic_string().c_str());
            err = POSIX_ENOENT;
            return 0;
        } else {
            // Create the file if it didn't exist and the create flag was specified
            std::ofstream temp = std::ofstream(host_path);
            temp.close();
        }
    }

    if (trunc) {
        // Discard existing content. We just recreate the file
        fs::remove(host_path);
        std::ofstream temp = std::ofstream(host_path);
        temp.close();
    }

    const u64 new_file_id = OS::requestHandle();
    open_files[new_file_id] = std::make_unique<File>();
    auto& file_desc = open_files[new_file_id];
    file_desc->path         = host_path;
    file_desc->guest_path   = path;
    file_desc->is_dir       = is_dir;
    file_desc->flags        = flags;

    FILE* file = nullptr;
    if (!is_dir) {
        file = std::fopen(host_path.generic_string().c_str(), mode.c_str());
        if (!file) {
            Helpers::panic("Failed to open file %s with mode %s\n", host_path.generic_string().c_str(), mode.c_str());
        }
    }
    else {
        // Add "." and ".." dirents
        SceKernelDirent dirent;
        dirent.d_fileno = fileno++;
        dirent.d_reclen = sizeof(SceKernelDirent);
        dirent.d_type   = SCE_KERNEL_DT_DIR;
        std::strcpy(dirent.d_name, ".");
        file_desc->dirents.push_back(dirent);

        dirent.d_fileno = fileno++;
        //dirent.d_reclen = sizeof(SceKernelDirent);
        //dirent.d_type = SCE_KERNEL_DT_DIR;
        std::strcpy(dirent.d_name, "..");
        file_desc->dirents.push_back(dirent);

        // Populate dirents
        for (auto& entry : fs::directory_iterator(host_path)) {
            auto name_str = entry.path().filename().generic_string();
            auto& dirent = file_desc->dirents.emplace_back();
            dirent.d_fileno = fileno++;  // TODO: Proper implementation?
            dirent.d_reclen = sizeof(SceKernelDirent);
            dirent.d_type = !entry.is_directory() ? SCE_KERNEL_DT_REG : SCE_KERNEL_DT_DIR;
            dirent.d_namlen = name_str.length();
            std::strcpy(dirent.d_name, name_str.c_str());
        }
    }

    file_desc->file = file;
    log("Opened file %s with id %d\n", host_path.generic_string().c_str(), new_file_id);
    return new_file_id;
}

u64 opendir(fs::path path) {
    const fs::path host_path = guestPathToHost(path);
    if (!fs::exists(host_path)) {
        log("WARNING: Tried to open non-existing dir %s\n", path.generic_string().c_str());
        return 0;
    }

    const u64 new_file_id = OS::requestHandle();
    open_dirs[new_file_id] = { path, 0 };
    log("Opened directory %s\n", path.generic_string().c_str());
    return new_file_id;
}

void close(u64 file_id) {
    auto& file = getFileFromID(file_id);

    if (!file.is_dir) {
        std::fclose(file.file);
    }
    open_files.erase(file_id);
}

void closedir(u64 file_id) {
    // TODO
}

u64 read(u64 file_id, u8* buf, u64 size) {
    // TODO: In ChonkyStation3, I had to make this function read only up to PAGE_SIZE bytes due to there being a distinction
    // between virtual and physical memory. I don't think this will be a problem here so I just read it all at once.

    auto& file = getFileFromID(file_id);
    if (file.is_dir) {
        Helpers::panic("FS::read: file is dir\n");
    }

    return std::fread(buf, sizeof(u8), size, file.file);
}

u64 write(u64 file_id, u8* buf, u64 size) {
    // TODO: Read the comment in the read function
    
    auto& file = getFileFromID(file_id);
    if (file.is_dir) {
        Helpers::panic("FS::write: file is dir\n");
    }

    return std::fwrite(buf, sizeof(u8), size, file.file);
}

u64 seek(u64 file_id, s64 offs, u32 mode) {
    Helpers::debugAssert(mode <= 3, "Filesystem: invalid seek mode");

    FILE* file = getFileFromID(file_id).file;
#ifdef _WIN32
    _fseeki64(file, offs, mode);
    return _ftelli64(file);
#else
    std::fseek(file, offs, mode);
    return std::ftell(file);
#endif
}

u64 tell(u64 file_id) {
    FILE* file = getFileFromID(file_id).file;
#ifdef _WIN32
    return _ftelli64(file);
#else
    return std::ftell(file);
#endif
}

// Returns false if path already exists
bool mkdir(fs::path path) {
    const fs::path host_path = guestPathToHost(path);
    if (fs::exists(host_path)) return false;

    fs::create_directories(host_path);
    return true;
}

std::unique_lock<std::mutex> getFileLock(u64 file_id) {
    auto& file = getFileFromID(file_id);
    return std::unique_lock<std::mutex>(file.mtx);
}

u64 getFileSize(u64 file_id) {
    auto& file = getFileFromID(file_id);
    
    // If the file doesn't exist but the flag to create it was specified, return 0 size
    if ((file.flags & SCE_KERNEL_O_CREAT) && !fs::exists(file.path)) {
        return 0;
    }
    
    return fs::file_size(file.path);
}

u64 getFileSize(fs::path path) {
    return fs::file_size(guestPathToHost(path));
}

bool isDirectory(u64 file_id) {
    auto& file = getFileFromID(file_id);
    return fs::is_directory(file.path);
}

bool isDirectory(fs::path path) {
    return fs::is_directory(guestPathToHost(path));
}

bool exists(u64 file_id) {
    return open_files.contains(file_id);
}

bool exists(fs::path path) {
    return fs::exists(guestPathToHost(path));
}

File& getFileFromID(u32 id) {
    if (!open_files.contains(id))
        Helpers::panic("File id %d does not exist\n", id);
    return *open_files[id];
}

Directory& getDirFromID(u32 id) {
    if (!open_dirs.contains(id))
        Helpers::panic("Dir id %d does not exist\n", id);
    return open_dirs[id];
}

bool isDeviceMounted(Device device) {
    return mounted_devices.contains(device);
}

bool isDeviceMounted(fs::path path) {
    return mounted_devices.contains(getDeviceFromPath(path));
}

fs::path guestPathToHost(fs::path path) {
    const std::string path_str = path.generic_string();
    fs::path guest_path;

    // Check if the path is valid
    // All paths on PS4 must be absolute
    if (path.begin()->generic_string() != "/") {
        log("Path %s is not valid\n", path.generic_string().c_str());
        return "";
    }

    // Check if device is valid
    std::string device_str = std::next(path.begin(), 1)->generic_string();
    Device device = stringToDevice(device_str);
    if (device == Device::INVALID)
        Helpers::panic("Path %s: device %s is not a valid device\n", path_str.c_str(), device_str.c_str());

    // Check if device is mounted
    if (!isDeviceMounted(device))
        Helpers::panic("Path %s: device %s is not mounted\n", path_str.c_str(), device_str.c_str());

    // Check if this path contains only the device
    int count = 0;
    for (const auto& i : path) count++;
    if (count == 2 || (count == 3 && path.filename().empty())) {
        return mounted_devices[device];
    }
    
    // Convert to host path
    std::string path_no_device_start = std::next(path.begin(), 2)->generic_string();
    fs::path path_no_device = path_str.substr(path_str.find(path_no_device_start));
    
    guest_path = mounted_devices[device] / path_no_device;
    return guest_path;
}

Device getDeviceFromPath(fs::path path) {
    std::string device_str = std::next(path.begin(), 1)->generic_string();
    return stringToDevice(device_str);
}

bool isValidDevice(fs::path path) {
    const auto device = getDeviceFromPath(path);
    return device != Device::INVALID;
}

std::string deviceToString(Device device) {
    if (device == Device::INVALID) Helpers::panic("deviceToString: invalid device\n");

    switch (device) {
    case Device::APP0:          return "app0";
    case Device::SAVEDATA0:     return "savedata0";
    case Device::DEV:           return "dev";
    }
}

Device stringToDevice(std::string device) {
    if      (device == "app0")          return Device::APP0;
    else if (device == "savedata0")     return Device::SAVEDATA0;
    else if (device == "dev")           return Device::DEV;
    else return Device::INVALID;
}

} // End namespace PS4::FS
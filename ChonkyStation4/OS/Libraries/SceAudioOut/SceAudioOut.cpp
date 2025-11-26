#include "SceAudioOut.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceAudioOut {

MAKE_LOG_FUNCTION(log, lib_sceAudioOut);

void init(Module& module) {
    module.addSymbolStub("JfEPXVxhFqA", "sceAudioOutInit", "libSceAudioOut", "libSceAudioOut");
    module.addSymbolStub("ekNvsT22rsY", "sceAudioOutOpen", "libSceAudioOut", "libSceAudioOut", 1);
    module.addSymbolStub("QOQtbeDqsT4", "sceAudioOutOutput", "libSceAudioOut", "libSceAudioOut", 1);
}

}   // End namespace PS4::OS::Libs::SceAudioOut
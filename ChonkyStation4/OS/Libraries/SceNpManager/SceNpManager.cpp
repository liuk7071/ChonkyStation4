#include "SceNpManager.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/UserManagement.hpp>
#include <OS/Libraries/Kernel/Kernel.hpp>
#include <OS/Libraries/SceNpMatching/SceNpMatching.hpp>
#include <OS/Libraries/SceNpWebApi/SceNpWebApi.hpp>


namespace PS4::OS::Libs::SceNpManager {

MAKE_LOG_FUNCTION(log, lib_sceNpManager);

static constexpr s32 SCE_NP_POLL_ASYNC_RET_FINISHED = 0;
static constexpr s32 SCE_NP_POLL_ASYNC_RET_RUNNING  = 1;

void init(Module& module) {
    module.addSymbolExport("3Zl8BePTh9Y", "sceNpCheckCallback", "libSceNpManager", "libSceNpManager", (void*)&sceNpCheckCallback);
    module.addSymbolExport("VfRSmPmj8Q8", "sceNpRegisterStateCallback", "libSceNpManager", "libSceNpManager", (void*)&sceNpRegisterStateCallback);
    module.addSymbolExport("uFJpaKNBAj4", "sceNpRegisterGamePresenceCallback", "libSceNpManager", "libSceNpManager", (void*)&sceNpRegisterGamePresenceCallback);
    module.addSymbolExport("eQH7nWPcAgc", "sceNpGetState", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetState);
    module.addSymbolExport("IPb1hd1wAGc", "sceNpGetGamePresenceStatus", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetGamePresenceStatus);
    module.addSymbolExport("p-o74CnoNzY", "sceNpGetNpId", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetNpId);
    module.addSymbolExport("Oad3rvY-NJQ", "sceNpHasSignedUp", "libSceNpManager", "libSceNpManager", (void*)&sceNpHasSignedUp);
    module.addSymbolExport("rbknaUjpqWo", "sceNpGetAccountIdA", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetAccountIdA);
    module.addSymbolExport("XDncXQIJUSk", "sceNpGetOnlineId", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetOnlineId);
    module.addSymbolExport("VgYczPGB5ss", "sceNpGetUserIdByAccountId", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetUserIdByAccountId);
    module.addSymbolExport("JT+t00a3TxA", "sceNpGetAccountCountryA", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetAccountCountryA);
    module.addSymbolExport("eiqMCt9UshI", "sceNpCreateAsyncRequest", "libSceNpManager", "libSceNpManager", (void*)&sceNpCreateAsyncRequest);
    module.addSymbolExport("uqcPJLWL08M", "sceNpPollAsync", "libSceNpManager", "libSceNpManager", (void*)&sceNpPollAsync);
    module.addSymbolExport("r6MyYJkryz8", "sceNpCheckPlus", "libSceNpManager", "libSceNpManager", (void*)&sceNpCheckPlus);
    module.addSymbolExport("ilwLM4zOmu4", "sceNpGetParentalControlInfo", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetParentalControlInfo);
    module.addSymbolExport("m9L3O6yst-U", "sceNpGetParentalControlInfoA", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetParentalControlInfoA);
    module.addSymbolExport("TPMbgIxvog0", "sceNpGetAccountLanguageA", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetAccountLanguageA);
    module.addSymbolExport("2rsFmlGWleQ", "sceNpCheckNpAvailability", "libSceNpManager", "libSceNpManager", (void*)&sceNpCheckNpAvailability);
    module.addSymbolExport("8Z2Jc5GvGDI", "sceNpCheckNpAvailabilityA", "libSceNpManager", "libSceNpManager", (void*)&sceNpCheckNpAvailabilityA);
    
    module.addSymbolStub("Ec63y59l9tw", "sceNpSetNpTitleId", "libSceNpManager", "libSceNpManager");
    //module.addSymbolStub("e-ZuhGEoeC4", "sceNpGetNpReachabilityState", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("A2CQ3kgSopQ", "sceNpSetContentRestriction", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("qQJfO8HAiaY", "sceNpRegisterStateCallbackA", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("oPO9U42YpgI", "sceNpGetGamePresenceStatusA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("mjjTXh+NHWY", "sceNpUnregisterStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("xViqJdDgKl0", "sceNpUnregisterPlusEventCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("hw5KNqAAels", "sceNpRegisterNpReachabilityStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("cRILAEvn+9M", "sceNpUnregisterNpReachabilityStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GImICnh+boA", "sceNpRegisterPlusEventCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("KswxLxk4c1Y", "sceNpRegisterGamePresenceCallbackA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GpLQDNKICac", "sceNpCreateRequest", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("S7QTn72PrDw", "sceNpDeleteRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("OzKvTvg3ZYU", "sceNpAbortRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GFhVUpRmbHE", "sceNpInGameMessageInitialize", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("TJqSgUEzexM", "sceNpSetNpTitleIdVsh", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("rT9Yk55JGho", "sceNpServiceCheckerIntInitialize", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("-Afi-JoRZ-U", "sceNpServiceChecker2IntInitialize", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("1F4yweQoqgg", "sceNpAppLaunchLinkIntInitialize", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("DK6xpBP1gxw", "sceNpAppLaunchLink2IntInitialize", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("l6Dl+2zlua0", "sceNpAppInfoIntInitialize", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("m87BHxt-H60", "sceNpGameIntentInitialize", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Gaxrp3EWY-M", "sceNpNotifyPlusFeature", "libSceNpManager", "libSceNpManager");

    module.addSymbolStub("AUuzKQIwhXY", "sceNpManagerIntAbortRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("HLQfohD2YuE", "sceNpManagerIntAccountId2UserId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("J0MUxuo9H9c", "sceNpManagerIntAddActiveSigninStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("wIX4m0mLfqA", "sceNpManagerIntAddOnlineIdChangeCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("E6rzFwsDFwE", "sceNpManagerIntAddPlusMemberTypeCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("S9xDus0Cums", "sceNpManagerIntAddSigninStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("m2PkI8YCJWc", "sceNpManagerIntAddUserStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("1aifBDr9oqc", "sceNpManagerIntAuthGetAuthorizationCode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("fMWCG0Tqofg", "sceNpManagerIntAuthGetIdToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("f6hCBlMb-6Q", "sceNpManagerIntBind", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Nx+FM+bz0ZQ", "sceNpManagerIntBindByJson", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uyo-bsZsxII", "sceNpManagerIntBindByJsonUserInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("vup0rbTEOPU", "sceNpManagerIntBindCompleted", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("eJDUHQBBwZM", "sceNpManagerIntBindGetWebAppToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("97RAfJch+qE", "sceNpManagerIntBindOfflineAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Xg7dJekKeHM", "sceNpManagerIntCheckGameNpAvailability", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("isNn0YyU83c", "sceNpManagerIntCheckGameNpAvailabilityA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("WtsuDMvVw-I", "sceNpManagerIntCheckGameNpAvailabilityWithPid", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("m4JiU8k2PyI", "sceNpManagerIntCheckNpAvailability", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("d+lmTLvsaRs", "sceNpManagerIntCheckNpAvailabilityByPid", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Dvk+xqAqXco", "sceNpManagerIntCheckNpState", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("U30AU92fWdU", "sceNpManagerIntCheckNpStateA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("fK5zqN0v5Qg", "sceNpManagerIntCheckPlus", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("mQU19DRkeyI", "sceNpManagerIntCheckSignin", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("meuwG2Ym0Pk", "sceNpManagerIntCheckTitlePatch", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("9fK1V0QGyL4", "sceNpManagerIntCheckTitleSystemUpdate", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("r7d8eEp5vJE", "sceNpManagerIntClearGameAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("5ZoFb+9L7LY", "sceNpManagerIntClearOnlineIdChangeFlag", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("3mR3QJFzrg4", "sceNpManagerIntClearParentalControlInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("yX8qSFmkiyc", "sceNpManagerIntClearParentalControlInfoSubAccount", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("bpF7OjR81T4", "sceNpManagerIntClearPlusMemberType", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("6TTRm8KRqbw", "sceNpManagerIntClearTicket", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("QZpXoz9wjbE", "sceNpManagerIntClearUsedFlag", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("miJIPnB2cfI", "sceNpManagerIntClearVshAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Wc3aCwYB5Go", "sceNpManagerIntClearVshToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("D2l3rQK-VzQ", "sceNpManagerIntClearVshTokenA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("IxqaB0MHl0s", "sceNpManagerIntCreateAuthenticationTicket", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("6n8NT1pHW9g", "sceNpManagerIntCreateLoginContext", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("CdQg39qlfgY", "sceNpManagerIntCreateLoginRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("xZk+QcivrFE", "sceNpManagerIntCreateRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("YpDwgqoKMaE", "sceNpManagerIntDeclareSystemProcess", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("EgmlHG93Tpw", "sceNpManagerIntDeleteLoginContext", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("HneC+SpeLwc", "sceNpManagerIntDeleteRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("tea1EXJiSB8", "sceNpManagerIntGetAccessTokenViaImplicitFlow", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("+RVCwHtA5kU", "sceNpManagerIntGetAccountAge", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("7+uKCMe4SLk", "sceNpManagerIntGetAccountCountry", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("fjJ4xXM+3Tw", "sceNpManagerIntGetAccountCountryA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("mUcn35JWAvI", "sceNpManagerIntGetAccountCountrySdk", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("dnyvPTam4Gc", "sceNpManagerIntGetAccountDateOfBirth", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("CConkVwc7Dc", "sceNpManagerIntGetAccountDateOfBirthA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("3TbxIy0VEiU", "sceNpManagerIntGetAccountDateOfBirthSdk", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("XS-eY7KRqjQ", "sceNpManagerIntGetAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("1H07-M8fGec", "sceNpManagerIntGetAccountIdSdk", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("C6xstRBFOio", "sceNpManagerIntGetAccountLanguage", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("e6rTjFmcQjY", "sceNpManagerIntGetAccountLanguageA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("HvNrMhlWBSk", "sceNpManagerIntGetAccountNpEnv", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("9lz4fkS+eEk", "sceNpManagerIntGetAccountType", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("UAA2-ZTmgJc", "sceNpManagerIntGetActiveSigninState", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("BoD6du5+wxo", "sceNpManagerIntGetAuthorizationCode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("1DMXuE0CbGQ", "sceNpManagerIntGetAuthorizationCodeA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("xPvV6oMKOWY", "sceNpManagerIntGetAuthorizationCodeWithPsnoUri", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("2VmnxS1aZG0", "sceNpManagerIntGetAuthorizationCodeWithRedirectUri", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("HkUgFhrpAD4", "sceNpManagerIntGetAuthServerErrorFlag", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("TXzpCgPmXEQ", "sceNpManagerIntGetClientCredentialAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("A3m-y8VVgqM", "sceNpManagerIntGetCommunicationRestrictionStatus", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("iTXe6EWAHek", "sceNpManagerIntGetGameAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("pJqrYc7v9Y4", "sceNpManagerIntGetGameAuthorizationCode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("RhjnQ70obPw", "sceNpManagerIntGetGameTicket", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("lY3vdIUZsG4", "sceNpManagerIntGetGameTicketWithPid", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("OcnSddPkQns", "sceNpManagerIntGetGameTitleBanInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("btKQfNe1jBY", "sceNpManagerIntGetGameTitleToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("O7ivIf9AIFI", "sceNpManagerIntGetGameTitleTokenA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("vfHBP2-WXcM", "sceNpManagerIntGetGameVshToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("pLjQLOflIUU", "sceNpManagerIntGetGameVshTokenWithPid", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("es6OiIxGiL0", "sceNpManagerIntGetIssuerId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("jCJEWuExbZg", "sceNpManagerIntGetLastAccountLanguage", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Oad+nopFTTA", "sceNpManagerIntGetMAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("BTRVfOx7K1c", "sceNpManagerIntGetNpEnv", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("azEmYv5NqWo", "sceNpManagerIntGetNpId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("gFB0RmKjyaI", "sceNpManagerIntGetNpIdByOnlineId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("9xcazelb3Ks", "sceNpManagerIntGetNpIdInternal", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("41CVMRinjWU", "sceNpManagerIntGetNpIdSdk", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("70Swvw7h6ck", "sceNpManagerIntGetOfflineAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("40GlkRTqRH0", "sceNpManagerIntGetOnlineId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("QnO8zMmKcGE", "sceNpManagerIntGetOnlineIdByAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("lYkDUwyzr0s", "sceNpManagerIntGetOnlineIdChangeFlag", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("jkQKWQTOu8g", "sceNpManagerIntGetOnlineIdInternal", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("sTtvF4QVhjg", "sceNpManagerIntGetOnlineIdSdk", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("FqtDOHUuDNw", "sceNpManagerIntGetParentalControlFlag", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("NS1sEhoj-B0", "sceNpManagerIntGetParentalControlInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("ggj9Qm4XDrU", "sceNpManagerIntGetParentalControlInfoA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("X52vXnVvtpE", "sceNpManagerIntGetParentalControlInfoNB", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("vrre3KW6OPg", "sceNpManagerIntGetPlusMemberType", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("XRFchqddEVU", "sceNpManagerIntGetPlusMemberTypeNB", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("iDlso2ZrQfA", "sceNpManagerIntGetServerError", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("6miba-pcQt8", "sceNpManagerIntGetSigninState", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("hnOWouVmZMY", "sceNpManagerIntGetSigninTelemetryInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("jce1Jhbmj7Q", "sceNpManagerIntGetTemporarySsoToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uVAfWmv+cc8", "sceNpManagerIntGetTicket", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("43B0lauksLY", "sceNpManagerIntGetTicketA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("vfmUxlocdUY", "sceNpManagerIntGetTicketNB", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("xY607JqjoPk", "sceNpManagerIntGetTitleToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("HsHttp1Ktm0", "sceNpManagerIntGetTitleTokenWithCheck", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("OZTedKNUeFU", "sceNpManagerIntGetUserIdByAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uxLmJ141PmA", "sceNpManagerIntGetUserIdByMAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("MDczH3SxE9Q", "sceNpManagerIntGetUserIdByNpId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Nhxy2NmQhbs", "sceNpManagerIntGetUserIdByOfflineAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uSLgWz8ohak", "sceNpManagerIntGetUserIdByOnlineId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("H33CwgKf4Rs", "sceNpManagerIntGetUserIdByOnlineIdSdk", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("l5SkPv2i+f8", "sceNpManagerIntGetUserInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("PL10NiZ0XNA", "sceNpManagerIntGetUserList", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("etZ84Rf3Urw", "sceNpManagerIntGetUserNum", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("dCvPEYm3gHk", "sceNpManagerIntGetUserState", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("mBTFixSxTzQ", "sceNpManagerIntGetVshAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("+waQfICfHaw", "sceNpManagerIntGetVshAccessTokenWithCheck", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("3f0ejg9vcE8", "sceNpManagerIntGetVshClientId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("mKGqVK1SwFk", "sceNpManagerIntGetVshToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("0yDdrIgqpjE", "sceNpManagerIntGetVshTokenA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("yrME3knbt8U", "sceNpManagerIntGetVshTokenNB", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("WNmE+qXnYtg", "sceNpManagerIntGetWebAppToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("mA34DPndHuk", "sceNpManagerIntGetWebAppTokenByRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("p0dBN8P6oQk", "sceNpManagerIntGetWebAuthorizationCode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("1XsGBdpfDsU", "sceNpManagerIntInit", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("SubkMx98zRY", "sceNpManagerIntInitInternal", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("koU-Duc1F-0", "sceNpManagerIntIsServerMaintenanceError", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("ossvuXednsc", "sceNpManagerIntIsSubAccount", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("o8qKkphzJr0", "sceNpManagerIntIsSubAccountByUserId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("FEYGFUlSCVQ", "sceNpManagerIntIsSystem", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("atgHp5dQi5k", "sceNpManagerIntIsTemporarySignout", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("jwOjEhWD6E4", "sceNpManagerIntIsUnregisteredClientError", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("aU5QaUCW-Ik", "sceNpManagerIntLoginAddJsonInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("KQYLX4tVLe4", "sceNpManagerIntLoginAuthenticate", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("bzf8a7LxtCQ", "sceNpManagerIntLoginBind", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("XRpM9tQecCU", "sceNpManagerIntLoginCheckSignin", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("xR8S67myUos", "sceNpManagerIntLoginCreateAuthenticationTicket", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("xAdGRA3ucDg", "sceNpManagerIntLoginGet2svInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("-P0LG2EUFBE", "sceNpManagerIntLoginGetAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("38cfkczfN08", "sceNpManagerIntLoginGetAccessTokenViaImplicitFlow", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("dvkqP9KUMfk", "sceNpManagerIntLoginGetAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("sEZaB9KQ10k", "sceNpManagerIntLoginGetAuthenticateResponse", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Y+hLqeLseRk", "sceNpManagerIntLoginGetAuthorizationCode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("EXeJ80p01gs", "sceNpManagerIntLoginGetDeviceCodeInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("yqsFy9yg2rU", "sceNpManagerIntLoginGetEmail", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("wXfHhmzUjK4", "sceNpManagerIntLoginGetOnlineId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("yWMBHiRdEbk", "sceNpManagerIntLoginGetUserId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("BdzbUHOEoO4", "sceNpManagerIntLoginGetWebAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("mgTL+L1bGqU", "sceNpManagerIntLoginGetWebAccessTokenByClientId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uaCfG0TAPmg", "sceNpManagerIntLoginParseJsonUserInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("yHl0pPA3rPQ", "sceNpManagerIntLoginResetSsoToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("0cLPZO1Voe8", "sceNpManagerIntLoginRevalidatePassword", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("hmVLIi3pQDE", "sceNpManagerIntLoginSetAccountInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("X-WHexCbxcI", "sceNpManagerIntLoginSetSsoToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("YC3k6Hcy9+E", "sceNpManagerIntLoginSetUserId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("rCnvauevHHc", "sceNpManagerIntLoginSignin", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("qmZHHehEDog", "sceNpManagerIntLoginValidateCredential", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("zXukItkUuko", "sceNpManagerIntLoginValidateKratosAuthCode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("ujtFwWJnv+E", "sceNpManagerIntLoginVerifyDeviceCode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("d8ZF6RxS4gg", "sceNpManagerIntMAccountId2UserId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("OxI6k0G6RlU", "sceNpManagerIntNotifyPlusFeature", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("gVW1ZB-uUj4", "sceNpManagerIntNpId2UserId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("XEPf4yyKUyk", "sceNpManagerIntOnlineId2NpId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("wHY17vvA+mA", "sceNpManagerIntOnlineId2UserId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("aBcwP392v0E", "sceNpManagerIntParseIdToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("Qk5ehOXWs4g", "sceNpManagerIntParseJsonUserInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("d0IkWV+u25g", "sceNpManagerIntPfAuth", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("WJV0RYOEc-E", "sceNpManagerIntPsnCoreInternalTest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("SuBDgQswXgo", "sceNpManagerIntRemoveActiveSigninStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("5nayeu8VK5Y", "sceNpManagerIntRemoveOnlineIdChangeCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("PafRf+sxnwA", "sceNpManagerIntRemovePlusMemberTypeCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("zh2KsQZlAN4", "sceNpManagerIntRemoveSigninStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("V6jvzQi5EQc", "sceNpManagerIntRemoveUserStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("k4M1w5Xstck", "sceNpManagerIntRevalidatePassword", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("OjoP+2xDY5c", "sceNpManagerIntRevalidatePasswordOld", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("96pmXKJwUWY", "sceNpManagerIntSetCheckPlusResult", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("ik86e1xLpoo", "sceNpManagerIntSetCommerceDialogPlusResult", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("C77VnsdaKKI", "sceNpManagerIntSetPlusMemberTypeNB", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("MTzWc9nxOy4", "sceNpManagerIntSetRequestHeader", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("PZhz+vjp2CM", "sceNpManagerIntSetTimeout", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("7uqf3hPw8Yw", "sceNpManagerIntSignin", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("6G6BrngbzRg", "sceNpManagerIntSigninByJson", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("z-I94rQUMRY", "sceNpManagerIntSigninByJsonUserInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("64D6V-ADQe0", "sceNpManagerIntSignout", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("HyrG1GT4JxI", "sceNpManagerIntStartSystemMode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("+IagDajB6AQ", "sceNpManagerIntSubmitUserCode", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("wUT4cOK0bj0", "sceNpManagerIntTemporarySignout", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("prZNuK3iTi8", "sceNpManagerIntTerm", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("H51PINVcMM8", "sceNpManagerIntTermInternal", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uZnL4QZxzkI", "sceNpManagerIntUnbind", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("IG6ZoGSDaMk", "sceNpManagerIntUnbindOfflineAccountId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("cOKT8SyGb2g", "sceNpManagerIntUpdateBcAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("dTvQe2clcNw", "sceNpManagerIntUpdateVshAccessToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("3FtD6y5Rk5Q", "sceNpManagerIntUpdateVshToken", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("8kM+eFzoBas", "sceNpManagerIntUpdateVshTokenA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("ljBQTMlrdfU", "sceNpManagerIntUserSignin", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("hwuMsTWU4Kg", "sceNpManagerIntUserSignout", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("jMRcqynQVRI", "sceNpManagerIntValidateCredential", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("qAoMArbG384", "sceNpManagerIntValidateQrCodeSession", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("6AcoqeEhs6E", "sceNpManagerIntWebLoginRequired", "libSceNpManager", "libSceNpManager");
    
    module.addSymbolStub("0c7HbXRKUt4", "sceNpRegisterStateCallbackForToolkit", "libSceNpManagerForToolkit", "libSceNpManager");
    module.addSymbolStub("YIvqqvJyjEc", "sceNpUnregisterStateCallbackForToolkit", "libSceNpManagerForToolkit", "libSceNpManager");
    module.addSymbolStub("JELHf4xPufo", "sceNpCheckCallbackForLib", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("JELHf4xPufo", "sceNpCheckCallbackForLib", "libSceNpManagerForToolkit", "libSceNpManager");
    
    module.addSymbolStub("VfRSmPmj8Q8", "sceNpRegisterStateCallback", "libSceNpManagerCompat", "libSceNpManager");
}

struct {
    SceNpStateCallback func = nullptr;
    void* userdata = nullptr;
} state_callback;

struct StateEvent {
    SceUserService::SceUserServiceUserId uid;
    SceNpState state;
    SceNpId np_id;
};
std::deque<StateEvent> state_events;
std::mutex state_event_mtx;

void pushStateEvent(const SceUserService::SceUserServiceUserId& uid, const SceNpState& state) {
    const std::unique_lock<std::mutex> lk(state_event_mtx);

    StateEvent event;
    event.uid = uid;
    event.state = state;
    sceNpGetNpId(uid, &event.np_id);
    state_events.push_back(event);
}

struct {
    SceNpGamePresenceCallback func = nullptr;
    void* userdata = nullptr;
} presence_callback;

struct PresenceEvent {
    SceNpOnlineId online_id;
    SceNpGamePresenceStatus status;
};
std::deque<PresenceEvent> presence_events;
std::mutex presence_event_mtx;

void pushPresenceEvent(const SceUserService::SceUserServiceUserId& uid, const SceNpGamePresenceStatus& status) {
    const std::unique_lock<std::mutex> lk(presence_event_mtx);

    PresenceEvent event;
    sceNpGetOnlineId(uid, &event.online_id);
    event.status = status;
    presence_events.push_back(event);
}

s32 PS4_FUNC sceNpCheckCallback() {
    log("sceNpCheckCallback()\n");

    {
        const std::unique_lock<std::mutex> lk(state_event_mtx);

        if (state_callback.func) {
            while (state_events.size()) {
                auto event = state_events.front();
                state_events.pop_front();

                // Does the callback expect the NpId to live after it returns? No right...?
                state_callback.func(event.uid, event.state, &event.np_id, state_callback.userdata);
            }
        }
    }

    {
        const std::unique_lock<std::mutex> lk(presence_event_mtx);

        if (presence_callback.func) {
            while (presence_events.size()) {
                auto event = presence_events.front();
                presence_events.pop_front();

                // Does the callback expect the OnlineId to live after it returns? No right...?
                presence_callback.func(&event.online_id, event.status, presence_callback.userdata);
            }
        }
    }

    SceNpMatching::checkCallback();
    SceNpWebApi::checkCallback();
    return SCE_OK;
}

s32 PS4_FUNC sceNpRegisterStateCallback(SceNpStateCallback callback, void* userdata) {
    log("sceNpRegisterStateCallback(callback=%p, userdata=%p)\n", callback, userdata);
    
    const std::unique_lock<std::mutex> lk(state_event_mtx);

    if (state_callback.func) {
        log("sceNpRegisterStateCallback: tried to register twice\n");
        return SCE_NP_ERROR_CALLBACK_ALREADY_REGISTERED;
    }

    state_callback.func     = callback;
    state_callback.userdata = userdata;
    return SCE_OK;   // TODO: Does this return SCE_OK or the callback id?
}

s32 PS4_FUNC sceNpRegisterGamePresenceCallback(SceNpGamePresenceCallback callback, void* userdata) {
    log("sceNpRegisterGamePresenceCallback(callback=%p, userdata=%p)\n", callback, userdata);

    const std::unique_lock<std::mutex> lk(presence_event_mtx);

    if (presence_callback.func) {
        log("sceNpRegisterGamePresenceCallback: tried to register twice\n");
        return SCE_NP_ERROR_CALLBACK_ALREADY_REGISTERED;
    }

    presence_callback.func      = callback;
    presence_callback.userdata  = userdata;
    return SCE_OK;   // TODO: Does this return SCE_OK or the callback id?
}

s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state) {
    log("sceNpGetState(uid=%d, state=*%p)\n", uid, state);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    *state = user->is_logged_in_psn ? SceNpState::SCE_NP_STATE_SIGNED_IN : SceNpState::SCE_NP_STATE_SIGNED_OUT;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetGamePresenceStatus(SceNpOnlineId* online_id, SceNpGamePresenceStatus* status) {
    log("sceNpGetGamePresenceStatus(online_id=\"%s\", status=*%p)\n", online_id->data, status);

    // TODO: lookup by online id
    *status = OS::User::current->is_logged_in_psn ? SceNpGamePresenceStatus::SCE_NP_GAME_PRESENCE_STATUS_ONLINE : SceNpGamePresenceStatus::SCE_NP_GAME_PRESENCE_STATUS_OFFLINE;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetNpId(SceUserService::SceUserServiceUserId uid, SceNpId* np_id) {
    log("sceNpGetNpId(uid=%d, np_id=*%p)\n", uid, np_id);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    if (!user->is_logged_in_psn)
        return SCE_NP_ERROR_SIGNED_OUT;

    // TODO: opt
    std::memset(np_id, 0, sizeof(SceNpId));
    std::strcpy(np_id->handle.data, user->online_id.c_str());
    return SCE_OK;
}

s32 PS4_FUNC sceNpHasSignedUp(SceUserService::SceUserServiceUserId uid, bool* has_signed_up) {
    log("sceNpHasSignedUp(uid=%d, has_signed_up=*%p)\n", uid, has_signed_up);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    *has_signed_up = user->is_logged_in_psn;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetAccountIdA(SceUserService::SceUserServiceUserId uid, SceNpAccountId* account_id) {
    log("sceNpGetAccountIdA(uid=%d, account_id=*%p)\n", uid, account_id);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    if (!user->is_logged_in_psn)
        return SCE_NP_ERROR_SIGNED_OUT;

    // Return dummy NpAccountId
    *account_id = user->account_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetOnlineId(SceUserService::SceUserServiceUserId uid, SceNpOnlineId* online_id) {
    log("sceNpGetOnlineId(uid=%d, online_id=*%p)\n", uid, online_id);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    if (!user->is_logged_in_psn)
        return SCE_NP_ERROR_SIGNED_OUT;

    std::memset(online_id, 0, sizeof(SceNpOnlineId));
    std::strcpy(online_id->data, user->online_id.c_str());
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetUserIdByAccountId(SceNpAccountId account_id, SceUserService::SceUserServiceUserId* uid) {
    log("sceNpGetUserIdByAccountId(account_id=%d, uid=*%p)\n", account_id, uid);

    // TODO
    *uid = 1;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetAccountCountryA(SceUserService::SceUserServiceUserId uid, SceNpCountryCode* country_code) {
    log("sceNpGetAccountCountryA(uid=%d, country_code=*%p)\n", uid, country_code);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    if (!user->is_logged_in_psn)
        return SCE_NP_ERROR_SIGNED_OUT;

    // Stubbed
    std::strncpy(country_code->data, "us", 2);
    country_code->term = '\0';
    country_code->padding[0] = 0;
    return SCE_OK;
}

s32 PS4_FUNC sceNpCreateRequest() {
    log("sceNpCreateRequest()\n");

    auto* req = OS::make<SceNpRequest>();
    req->is_async = false;
    return req->handle;
}

s32 PS4_FUNC sceNpCreateAsyncRequest(const SceNpCreateAsyncRequestParameter* param) {
    log("sceNpCreateAsyncRequest(param=*%p)\n", param);
    
    auto* req = OS::make<SceNpRequest>();
    req->is_async = true;
    req->state = SceNpRequest::State::Running;
    return req->handle;
}

s32 PS4_FUNC sceNpPollAsync(s32 req_id, s32* result) {
    log("sceNpPollAsync(req_id=%d, result=*%p)\n", req_id, result);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    if (!req->is_async) {
        Helpers::panic("sceNpPollAsync: specified request was not async\n");
    }

    const bool is_req_done = req->state == SceNpRequest::State::Finished;
    if (is_req_done) {
        *result = req->result;
    }

    return is_req_done ? SCE_NP_POLL_ASYNC_RET_FINISHED : SCE_NP_POLL_ASYNC_RET_RUNNING;
}

s32 PS4_FUNC sceNpCheckPlus(s32 req_id, const SceNpCheckPlusParameter* param, SceNpCheckPlusResult* result) {
    log("sceNpCheckPlus(req_id=%d, param=*%p, result=*%p)\n", req_id, param, result);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    result->authorized = true;

    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = SCE_OK;
    }
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetParentalControlInfo(s32 req_id, SceNpOnlineId* online_id, s8* age, SceNpParentalControlInfo* info) {
    log("sceNpGetParentalControlInfo(req_id=%d, online_id=\"%s\", age=*%p, info=*%p)\n", req_id, online_id->data, age, info);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    *age = 18;
    info->content_restriction   = false;
    info->chat_restriction      = false;
    info->ugc_restriction       = false;
    
    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = SCE_OK;
    }
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetParentalControlInfoA(s32 req_id, SceUserService::SceUserServiceUserId uid, s8* age, SceNpParentalControlInfo* info) {
    log("sceNpGetParentalControlInfoA(req_id=%d, uid=%d, age=*%p, info=*%p)\n", req_id, uid, age, info);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    *age = 18;
    info->content_restriction = false;
    info->chat_restriction = false;
    info->ugc_restriction = false;

    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = SCE_OK;
    }
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetAccountLanguageA(s32 req_id, SceUserService::SceUserServiceUserId uid, s8* age, SceNpLanguageCode* lang_code) {
    log("sceNpGetAccountLanguageA(req_id=%d, uid=%d, age=*%p, lang_code=*%p)\n", req_id, uid, age, lang_code);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    std::strcpy(lang_code->code, "en");

    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = SCE_OK;
    }
    return SCE_OK;
}

s32 PS4_FUNC sceNpCheckNpAvailability(s32 req_id, const Np::SceNpOnlineId* online_id, void* reserved) {
    log("sceNpCheckNpAvailability(req_id=%d, online_id=*%p, reserved=%p)\n", req_id, online_id, reserved);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    auto ret = SCE_OK;

    // TODO: Find user by online id
    auto* user = User::current;

    if (!user) {
        ret = SCE_NP_ERROR_USER_NOT_FOUND;
    }
    else if (!user->is_logged_in_psn) {
        ret = SCE_NP_ERROR_SIGNED_OUT;
    }

    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = ret;
    }

    return ret;
}

s32 PS4_FUNC sceNpCheckNpAvailabilityA(s32 req_id, SceUserService::SceUserServiceUserId uid) {
    log("sceNpCheckAvailabilityA(req_id=%d, uid=%d)\n", req_id, uid);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    auto ret = SCE_OK;
    
    auto* user = User::getUser(uid);
    if (!user) {
        ret = SCE_NP_ERROR_USER_NOT_FOUND;
    }
    else if (!user->is_logged_in_psn) {
        ret = SCE_NP_ERROR_SIGNED_OUT;
    }

    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = ret;
    }
    return ret;
}

}   // End namespace PS4::OS::Libs::SceNpManager
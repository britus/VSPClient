// ********************************************************************
// VSPLoadModel.m - VSPDriver user setup/install
//
// Copyright © 2025 by EoF Software Labs
// Copyright © 2024 Apple Inc. (some copied parts)
// SPDX-License-Identifier: MIT
// ********************************************************************
#import <os/log.h>

#import <Foundation/Foundation.h>
#import <SystemExtensions/SystemExtensions.h>
#import <SystemConfiguration/SystemConfiguration.h>

#import <vsploadermodel.h>
#import <vspsmloader.h>

extern void onDidFailWithError(uint32_t code, const char* message);
extern void onDidFinishWithResult(uint32_t code, const char* message);
extern void onNeedsUserApproval();

@implementation VSPLoaderModel {
    NSString *_dextIdentifier;
    BOOL _isUserUnload;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _dextIdentifier = @"org.eof.tools.VSPDriver";
        _state = VSPSmLoaderStateUnknown;

        // System Extension Properties Check
        if (@available(macOS 12, *)) {
            OSSystemExtensionRequest *request = [
                OSSystemExtensionRequest propertiesRequestForExtension:_dextIdentifier
                    queue:dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0)];
            request.delegate = self;
            [[OSSystemExtensionManager sharedManager] submitRequest:request];
        }
    }
    return self;
}

- (void)activateMyDext {
    [self activateExtension:_dextIdentifier];
}
- (void)removeMyDext {
    [self deactivateExtension:_dextIdentifier];
}

// State descriptions analogous to Swift version
- (NSString *)dextLoadingState {
    switch (self.state) {
        case VSPSmLoaderStateUnknown:
            return @"VSPSmLoaderStateUnknown";
        case VSPSmLoaderStateUnloaded:
            return @"VSPSmLoaderStateUnloaded";
        case VSPSmLoaderStateActivating:
            return @"VSPSmLoaderStateActivating";
        case VSPSmLoaderStateNeedsApproval:
            return @"VSPSmLoaderStateNeedsApproval";
        case VSPSmLoaderStateActivated:
            return @"VSPSmLoaderStateActivated";
        case VSPSmLoaderStateActivationError:
            return @"VSPSmLoaderStateActivationError";
        case VSPSmLoaderStateRemoval:
            return @"VSPSmLoaderStateRemoval";
    }
}

- (void)activateExtension:(NSString *)dextIdentifier {
    _isUserUnload = NO;

    if (@available(macOS 10.15, *)) {
        OSSystemExtensionRequest *request = [
        OSSystemExtensionRequest activationRequestForExtension:dextIdentifier
                                 queue:dispatch_get_main_queue()];
    request.delegate = self;
        [[OSSystemExtensionManager sharedManager] submitRequest:request];
    } else {
        // Fallback on earlier versions
        return;
    }

    self.state = [VSPSmLoader processState:self.state
                                 withEvent:VSPSmLoaderEventActivationStarted];
}

- (void)deactivateExtension:(NSString *)dextIdentifier {
    _isUserUnload = YES;

    if (@available(macOS 10.15, *)) {
        OSSystemExtensionRequest *request = [
        OSSystemExtensionRequest deactivationRequestForExtension:dextIdentifier
                                 queue:dispatch_get_main_queue()];
        request.delegate = self;
        [[OSSystemExtensionManager sharedManager] submitRequest:request];
        self.state = [VSPSmLoader processState:self.state
                                     withEvent:VSPSmLoaderEventUninstallStarted];
    } else {
        onDidFailWithError(0xf0001015, "At least MacOS 10.15 or later required.");
        return;
    }
}

// Implement OSSystemExtensionRequestDelegate methods...

- (OSSystemExtensionReplacementAction)request:(nonnull OSSystemExtensionRequest *)request
                  actionForReplacingExtension:(nonnull OSSystemExtensionProperties *)existing
                                withExtension:(nonnull OSSystemExtensionProperties *)extension
API_AVAILABLE(macos(10.15))
{
    os_log(OS_LOG_DEFAULT, "[VSPLM] Got the upgrade request (%@ -> %@); answering replace",
           existing.bundleVersion, extension.bundleVersion);
    return OSSystemExtensionReplacementActionReplace;
}

- (void)request:(nonnull OSSystemExtensionRequest *)request didFailWithError:(nonnull NSError *)error  API_AVAILABLE(macos(10.15)){
    os_log(OS_LOG_DEFAULT, "[VSPLM] %s", error.description.UTF8String);
    onDidFailWithError(error.code, error.description.UTF8String);
}

- (void)request:(nonnull OSSystemExtensionRequest *)request didFinishWithResult:(OSSystemExtensionRequestResult)result  API_AVAILABLE(macos(10.15)){
    if (result == OSSystemExtensionRequestCompleted) {
        os_log(OS_LOG_DEFAULT, "[VSPLM] Installation successfully.");
    }
    else if (result == OSSystemExtensionRequestWillCompleteAfterReboot) {
        os_log(OS_LOG_DEFAULT, "[VSPLM] Installation pending. Activate after reboot!");
    } else {
        os_log(OS_LOG_DEFAULT, "[VSPLM] Installation status: %d", (int)result);
    }
    onDidFinishWithResult((uint32_t)result, "Installation finished");
}

- (void)requestNeedsUserApproval:(nonnull OSSystemExtensionRequest *)request  API_AVAILABLE(macos(10.15)){
    os_log(OS_LOG_DEFAULT, "[VSPLM] Require user approval.");
    onNeedsUserApproval();
}

@end

// ********************************************************************
// VSPLoadModel.m - VSPDriver user setup/install
//
// Copyright © 2025 by EoF Software Labs
// Copyright © 2024 Apple Inc. (some copied parts)
// SPDX-License-Identifier: MIT
// ********************************************************************
#import <Foundation/Foundation.h>
#import <SystemExtensions/SystemExtensions.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <vsploadermodel.h>
#import <vspsmloader.h>

extern void onDidFailWithError(uint64_t code, const char* message);
extern void onDidFinishWithResult(uint64_t code, const char* message);
extern void onNeedsUserApproval(void);

@implementation VSPLoaderModel {
    NSString* _dextBundleId;
    BOOL      _isUserUnload;
    uint64_t  _status;
}

- (instancetype)init:(const char*)dextBundleId
{
    self = [super init];
    if (self) {
        _status = 0xf0000000;
        _state = VSPSmLoaderStateUnknown;
        _dextBundleId = [NSString stringWithCString:dextBundleId encoding:NSUTF8StringEncoding];
        if (_dextBundleId != NULL) {
            fprintf(stdout, "[VSPLM] Using bundle Id: %s\n", _dextBundleId.UTF8String);
            // System Extension Properties Check
            if (@available(macOS 12, *)) {
                OSSystemExtensionRequest *request = [
                    OSSystemExtensionRequest propertiesRequestForExtension:_dextBundleId
                        queue:dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0)];
                request.delegate = self;
                [[OSSystemExtensionManager sharedManager] submitRequest:request];
            }
        }
    }
    return self;
}

- (void)activateMyDext
{
    [self activateExtension:_dextBundleId];
}
- (void)removeMyDext
{
    [self deactivateExtension:_dextBundleId];
}

// State descriptions analogous to Swift version
- (NSString *)dextLoadingState
{
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

- (void)activateExtension:(NSString *)dextBundleId
{
    _isUserUnload = NO;
    _status = 0xf1000000;

    if (@available(macOS 10.15, *)) {
        OSSystemExtensionRequest *request = [
        OSSystemExtensionRequest activationRequestForExtension:dextBundleId
                                 queue:dispatch_get_main_queue()];
        request.delegate = self;
        [[OSSystemExtensionManager sharedManager] submitRequest:request];

        self.state = [VSPSmLoader processState:self.state
                                     withEvent:VSPSmLoaderEventActivationStarted];
    } else {
        onDidFailWithError(0xe1001015, "At least MacOS 10.15 or later required.");
        return;
    }
}

- (void)deactivateExtension:(NSString *)dextBundleId
{
    _isUserUnload = YES;
    _status = 0xf2000000;

    if (@available(macOS 10.15, *)) {
        OSSystemExtensionRequest *request = [
        OSSystemExtensionRequest deactivationRequestForExtension:dextBundleId
                                 queue:dispatch_get_main_queue()];
        request.delegate = self;
        [[OSSystemExtensionManager sharedManager] submitRequest:request];
        self.state = [VSPSmLoader processState:self.state
                                     withEvent:VSPSmLoaderEventUninstallStarted];
    } else {
        onDidFailWithError(0xe1001015, "At least MacOS 10.15 or later required.");
        return;
    }
}

// Implement OSSystemExtensionRequestDelegate methods...

- (OSSystemExtensionReplacementAction)request:(nonnull OSSystemExtensionRequest *)request
                  actionForReplacingExtension:(nonnull OSSystemExtensionProperties *)existing
                                withExtension:(nonnull OSSystemExtensionProperties *)extension
                                API_AVAILABLE(macos(10.15))
{
    fprintf(stdout, "[VSPLM] Got the upgrade request (%s -> %s); answering replace.\n",
           existing.bundleVersion.UTF8String, extension.bundleVersion.UTF8String);
    return OSSystemExtensionReplacementActionReplace;
}

- (void)request:(nonnull OSSystemExtensionRequest *)request
        didFailWithError:(nonnull NSError *)error API_AVAILABLE(macos(10.15))
{
    fprintf(stdout, "[VSPLM] Error 0x%lx %s\n", error.code, error.description.UTF8String);
    onDidFailWithError(error.code, error.description.UTF8String);
}

- (void)request:(nonnull OSSystemExtensionRequest *)request
        didFinishWithResult:(OSSystemExtensionRequestResult)result
        API_AVAILABLE(macos(10.15))
{
    _status |= result;

    if (result == OSSystemExtensionRequestCompleted) {
        fprintf(stdout, "[VSPLM] OS system extension request completed.\n");
        if (result == 0) {
            if (self.state == VSPSmLoaderStateRemoval) {
                onDidFinishWithResult(_status, "Driver successfully removed.");
            }
            else {
                onDidFinishWithResult(_status, "Driver successfully activated.");
            }
        }
        else {
            onDidFinishWithResult(_status, "Wait for VSP driver activation.");
        }
    }
    else if (result == OSSystemExtensionRequestWillCompleteAfterReboot) {
        fprintf(stdout, "[VSPLM] OS system extension request will complete after reboot.\n");
        if (self.state == VSPSmLoaderStateRemoval) {
            fprintf(stdout, "[VSPLM] Removal pending. Removed after reboot.\n");
            onDidFinishWithResult(_status, "Will remove VSP driver after reboot.");
        }
        else {
            fprintf(stdout, "[VSPLM] Installation pending. Activate after reboot.\n");
            onDidFinishWithResult(_status, "Activate VSP driver after reboot.");
        }
    }
    else {
        fprintf(stdout, "[VSPLM] System Extension request status: %d\n", (int)result);
        if (self.state == VSPSmLoaderStateRemoval) {
            onDidFinishWithResult(_status, "Remove VSP driver with status.");
        }
        else {
            onDidFinishWithResult(_status, "Activate VSP driver with status.");
        }
    }
}

- (void)requestNeedsUserApproval:(nonnull OSSystemExtensionRequest *)request
        API_AVAILABLE(macos(10.15))
{
    fprintf(stdout, "[VSPLM] Require user approval.\n");
    onNeedsUserApproval();
}

@end

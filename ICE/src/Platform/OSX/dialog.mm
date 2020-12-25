#include <Platform/dialog.h>
#import <Cocoa/Cocoa.h>

const std::string open_native_dialog(std::string const& filter) {
    // Create the File Open Dialog class.
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    // Enable the selection of files in the dialog.
    [openDlg setCanChooseFiles:YES];

    // Multiple files not allowed
    [openDlg setAllowsMultipleSelection:NO];

    // Can't select a directory
    [openDlg setCanChooseDirectories:NO];

    //openDlg.allowedFileTypes = @[[NSString stringWithUTF8String:filter.c_str()]];

    // Display the dialog. If the OK button was pressed,
    // Process the files.
    NSString* url = @"";
    if ([openDlg runModal] == NSModalResponseOK) {
        NSArray* urls = [openDlg URLs];
        url = [urls[0] path];
    }
    return std::string(url.UTF8String);
}

const std::string open_native_folder_dialog() {
    // Create the File Open Dialog class.
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    [openDlg setCanChooseFiles:YES];
    [openDlg setAllowsMultipleSelection:NO];
    [openDlg setCanChooseDirectories:YES];
    [openDlg setCanChooseFiles:NO];
    [openDlg setCanCreateDirectories:YES];
    NSString* url = @"";
    if ([openDlg runModal] == NSModalResponseOK) {
        NSArray* urls = [openDlg URLs];
        url = [urls[0] path];
    }
    return std::string(url.UTF8String);
}
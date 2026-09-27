#import <UIKit/UIKit.h>
#include <SDL3/SDL.h>
#include "ios_launcher.h"
#include "platform/ios/window_orientation.h"
#include <stdexcept>

@interface EntisLibraryController : UITableViewController <UIDocumentPickerDelegate>
@property(nonatomic, strong) NSURL* gamesDirectory;
@property(nonatomic, strong) NSArray<NSURL*>* games;
@property(nonatomic, copy) NSString* selectedGame;
@property(nonatomic, strong) UILabel* instructions;
@property(nonatomic) BOOL importing;
@end

@implementation EntisLibraryController
- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"EntisGLS Launcher";
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc]
        initWithTitle:@"Import folder" style:UIBarButtonItemStylePlain
        target:self action:@selector(importFolder)];
    self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc]
        initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh target:self action:@selector(reloadGames)];
    self.instructions = [UILabel new];
    self.instructions.numberOfLines = 0;
    self.instructions.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    self.instructions.adjustsFontForContentSizeCategory = YES;
    self.instructions.textColor = UIColor.secondaryLabelColor;
    UIView* header = [UIView new];
    [header addSubview:self.instructions];
    self.tableView.tableHeaderView = header;
    [self reloadGames];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reloadGames)
        name:UIApplicationDidBecomeActiveNotification object:nil];
}
- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    const CGFloat width = self.tableView.bounds.size.width;
    CGSize size = [self.instructions sizeThatFits:CGSizeMake(MAX(1, width - 40), CGFLOAT_MAX)];
    self.instructions.frame = CGRectMake(20, 16, MAX(1, width - 40), size.height);
    UIView* header = self.tableView.tableHeaderView;
    CGRect frame = CGRectMake(0, 0, width, size.height + 32);
    if (!CGRectEqualToRect(header.frame, frame)) {
        header.frame = frame;
        self.tableView.tableHeaderView = header;
    }
}
- (void)showError:(NSString*)message {
    UIAlertController* alert = [UIAlertController alertControllerWithTitle:@"Import failed"
        message:message preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}
- (void)reloadGames {
    if (self.importing) return;
    NSError* error = nil;
    NSArray<NSURL*>* contents = [NSFileManager.defaultManager contentsOfDirectoryAtURL:self.gamesDirectory
        includingPropertiesForKeys:@[NSURLIsDirectoryKey, NSURLIsSymbolicLinkKey]
        options:NSDirectoryEnumerationSkipsHiddenFiles error:&error];
    NSMutableArray<NSURL*>* games = [NSMutableArray new];
    for (NSURL* url in contents) {
        NSNumber* directory = nil;
        NSNumber* symlink = nil;
        [url getResourceValue:&directory forKey:NSURLIsDirectoryKey error:nil];
        [url getResourceValue:&symlink forKey:NSURLIsSymbolicLinkKey error:nil];
        if (directory.boolValue && !symlink.boolValue) [games addObject:url];
    }
    [games sortUsingComparator:^NSComparisonResult(NSURL* left, NSURL* right) {
        return [left.lastPathComponent localizedStandardCompare:right.lastPathComponent];
    }];
    self.games = games;
    self.instructions.text = error ? error.localizedDescription :
        @"Select a game below, or import its complete, extracted folder from Files.\n\n"
         "You can also copy game folders into On My iPhone / EntisGLS Launcher / Games, then tap Refresh.\n\n"
         "No games are included. This launcher supports the traditional Cotopha CSX runtime; other engines and native plugins may not work.";
    [self.tableView reloadData];
    [self.view setNeedsLayout];
}
- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return self.games.count;
}
- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"game"];
    if (!cell) cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:@"game"];
    cell.textLabel.text = self.games[indexPath.row].lastPathComponent;
    cell.textLabel.numberOfLines = 0;
    cell.textLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    cell.detailTextLabel.text = @"Tap to launch";
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    return cell;
}
- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    if (!self.importing) self.selectedGame = self.games[indexPath.row].path;
}
- (void)importFolder {
    UIDocumentPickerViewController* picker = [[UIDocumentPickerViewController alloc]
        initWithDocumentTypes:@[@"public.folder"] inMode:UIDocumentPickerModeOpen];
    picker.delegate = self;
    picker.allowsMultipleSelection = NO;
    [self presentViewController:picker animated:YES completion:nil];
}
- (void)documentPicker:(UIDocumentPickerViewController*)controller didPickDocumentsAtURLs:(NSArray<NSURL*>*)urls {
    NSURL* source = urls.firstObject;
    if (!source) return;
    // Hold the grant through the background coordinated copy, then release it.
    const BOOL scoped = [source startAccessingSecurityScopedResource];
    NSString* sourcePath = source.URLByResolvingSymlinksInPath.path;
    NSString* gamesPath = self.gamesDirectory.URLByResolvingSymlinksInPath.path;
    if ([gamesPath isEqualToString:sourcePath] || [gamesPath hasPrefix:[sourcePath stringByAppendingString:@"/"]]) {
        if (scoped) [source stopAccessingSecurityScopedResource];
        [self showError:@"Select a single game folder, rather than this app's Documents or Games folder."];
        return;
    }
    self.importing = YES;
    self.navigationItem.rightBarButtonItem.enabled = NO;
    self.navigationItem.leftBarButtonItem.enabled = NO;
    self.tableView.allowsSelection = NO;
    self.instructions.text = @"Copying the game folder into this app… Keep the app open until the import finishes. Large games may take several minutes.";
    [self.view setNeedsLayout];
    NSURL* gamesDirectory = self.gamesDirectory;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        @autoreleasepool {
            NSFileManager* files = [NSFileManager new];
            __block NSError* copyError = nil;
            NSError* coordinationError = nil;
            NSURL* temporary = [gamesDirectory URLByAppendingPathComponent:
                [@".import-" stringByAppendingString:NSUUID.UUID.UUIDString] isDirectory:YES];
            NSFileCoordinator* coordinator = [[NSFileCoordinator alloc] initWithFilePresenter:nil];
            [coordinator coordinateReadingItemAtURL:source options:0 error:&coordinationError
                byAccessor:^(NSURL* readable) {
                    NSNumber* directory = nil;
                    [readable getResourceValue:&directory forKey:NSURLIsDirectoryKey error:&copyError];
                    if (!directory.boolValue) {
                        if (!copyError) copyError = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileReadUnknownError
                            userInfo:@{NSLocalizedDescriptionKey: @"Choose an extracted game folder."}];
                        return;
                    }
                    [files copyItemAtURL:readable toURL:temporary error:&copyError];
                }];
            if (scoped) [source stopAccessingSecurityScopedResource];
            NSError* error = coordinationError ?: copyError;
            if (!error) {
                // Never overwrite an existing import or the user's original.
                NSString* name = source.lastPathComponent.length ? source.lastPathComponent : @"Game";
                NSURL* destination = [gamesDirectory URLByAppendingPathComponent:name isDirectory:YES];
                for (NSUInteger suffix = 2; [files fileExistsAtPath:destination.path]; ++suffix) {
                    destination = [gamesDirectory URLByAppendingPathComponent:
                        [NSString stringWithFormat:@"%@ (%lu)", name, (unsigned long)suffix] isDirectory:YES];
                }
                [files moveItemAtURL:temporary toURL:destination error:&error];
            }
            if (error) [files removeItemAtURL:temporary error:nil];
            dispatch_async(dispatch_get_main_queue(), ^{
                self.importing = NO;
                self.navigationItem.rightBarButtonItem.enabled = YES;
                self.navigationItem.leftBarButtonItem.enabled = YES;
                self.tableView.allowsSelection = YES;
                [self reloadGames];
                if (error) [self showError:error.localizedDescription];
            });
        }
    });
}
@end

namespace study::platform::sdl {
bool ChooseIOSLibraryGame(const std::string& documents, std::string& game, double smokeSeconds) {
    @autoreleasepool {
        NSURL* documentsURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:documents.c_str()] isDirectory:YES];
        NSURL* gamesURL = [documentsURL URLByAppendingPathComponent:@"Games" isDirectory:YES];
        NSError* error = nil;
        if (![NSFileManager.defaultManager createDirectoryAtURL:gamesURL withIntermediateDirectories:YES
                attributes:nil error:&error]) {
            throw std::runtime_error(std::string("Cannot create game library: ") + error.localizedDescription.UTF8String);
        }
        EntisLibraryController* library = [[EntisLibraryController alloc] initWithStyle:UITableViewStyleInsetGrouped];
        library.gamesDirectory = gamesURL;
        UIWindow* window = nil;
        for (UIScene* scene in UIApplication.sharedApplication.connectedScenes) {
            if ([scene isKindOfClass:UIWindowScene.class] && scene.activationState != UISceneActivationStateUnattached) {
                window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];
                break;
            }
        }
        if (!window) window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
        window.rootViewController = [[UINavigationController alloc] initWithRootViewController:library];
        [window makeKeyAndVisible];
        RefreshIOSLibraryOrientation((__bridge void*)window);
        SDL_Log("IOS_LIBRARY_READY documents=%s", documents.c_str());
        const Uint64 started = SDL_GetTicks();
        bool running = true;
        while (running && !library.selectedGame) {
            // SDL's UIKit PumpEvents services both the main and tracking run
            // loops, including the native Files picker and table scrolling.
            SDL_Event event;
            while (SDL_PollEvent(&event)) if (event.type == SDL_EVENT_QUIT) running = false;
            if (smokeSeconds > 0 && SDL_GetTicks() - started >= smokeSeconds * 1000) break;
            SDL_Delay(10);
        }
        if (library.selectedGame) game = library.selectedGame.UTF8String;
        const bool selected = library.selectedGame != nil;
        window.hidden = YES;
        window.rootViewController = nil;
        return selected;
    }
}
}

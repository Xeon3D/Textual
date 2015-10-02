/* ********************************************************************* 
                  _____         _               _
                 |_   _|____  _| |_ _   _  __ _| |
                   | |/ _ \ \/ / __| | | |/ _` | |
                   | |  __/>  <| |_| |_| | (_| | |
                   |_|\___/_/\_\\__|\__,_|\__,_|_|

 Copyright (c) 2010 - 2015 Codeux Software, LLC & respective contributors.
        Please see Acknowledgements.pdf for additional information.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Textual and/or "Codeux Software, LLC", nor the 
      names of its contributors may be used to endorse or promote products 
      derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

 *********************************************************************** */

#import "TextualApplication.h"

@class THOPluginDidReceiveServerInputConcreteObject;
@class THOPluginDidPostNewMessageConcreteObject;

/* All THOPluginProtocol messages are called within the primary class of a plugin and
 no where else. The primary class can be defined in the Info.plist of your bundle. The
 primary class acts similiar to an application delegate whereas it is responsible for 
 the lifetime management of your plugin. */

/* Each plugin has access to the global variables [self worldController] and 
 [self masterController] which both have unrestricted access to every component of 
 Textual. There is no need to store pointers in your plugin for these. They are always
 available just by calling the above mentioned method names. */

#pragma mark -
#pragma mark Localization

/* TPILocalizedString allows a plugin to use localized text within the plugin itself
 using Textual's own API. TPILocalizedString takes a two paramaters and that is the
 key to look inside the .strings file for and the formatting values.  */
/* This call expects the localized strings to be inside the filename "BasicLanguage.strings"
 Any other name will not work unless the actual Cocoa APIs for accessing localized strings
 is used in place of these. */
#define TPILocalizedString(k, ...)		TXLocalizedStringAlternative(TPIBundleFromClass(), k, ##__VA_ARGS__)

/*!
 * @brief Returns the NSBundle that owns the calling class.
 */
#define TPIBundleFromClass()				[NSBundle bundleForClass:[self class]]

/*!
 * A plugin must declare the minimum version of Textual that it is compatible with.
 *
 * Textual declares the constant named THOPluginProtocolCompatibilityMinimumVersion.
 * This constant is compared against the minimum version that a plugin specifies.
 * If the plugin's value is equal to or greater than this constant, then the plugin
 * is considered safe to load. 
 *
 * Unlike the version information that visible to the end user, this constant does
 * not change often. It only changes when modifications have been made to Textual’s
 * codebase that may result in crashes when loading existing plugins.
 *
 * For example, even though Textual’s visible version number is “5.0.4”, the value
 * of this constant is “5.0.0”
 *
 * To declare compatibility, add a new entry to a plugin's Info.plist file with 
 * the key named: "MinimumTextualVersion" - Set the value of this entry, as a 
 * String, to the return value of THOPluginProtocolCompatibilityMinimumVersion.
 *
 * @return "5.0.0" as of March 09, 2015
 */
TEXTUAL_EXTERN NSString * const THOPluginProtocolCompatibilityMinimumVersion;

@protocol THOPluginProtocol <NSObject>

@optional

#pragma mark -
#pragma mark Subscribed Events 

/*!
 * @brief Defines a list of commands that the plugin will support as user input 
 *  from the main text field.
 *
 * @return An NSArray containing a lowercase list of commands that the plugin
 *  will support as user input from the main text field.
 *
 * @discussion Considerations:
 * 
 * 1. If a command is a number (0-9), then insert it into the array as
 *  an NSString.
 *
 * 2. If a plugin tries to add a command already built into Textual onto
 *  this list, it will not work.
 *
 * 3. It is possible, but unlikely, that another plugin the end user has
 *  loaded is subscribed to the same command. When that occurs, each plugin
 *  subscribed to the command will be informed of when the command is performed.
 *
 * 4. To avoid conflicts, a plugin cannot subscribe to a command already
 *  defined by a script. If a script and a plugin both share the same command, then
 *  neither will be executed and an error will be printed to the OS X console.
 */
@property (nonatomic, readonly, copy) NSArray *subscribedUserInputCommands;

/*!
 * @brief Method invoked when a subscribed user input command requires processing.
 *
 * @param client The client responsible for the event
 * @param commandString The name of the command used by the end user
 * @param messageString Data that follows commandString
 */
- (void)userInputCommandInvokedOnClient:(IRCClient *)client commandString:(NSString *)commandString messageString:(NSString *)messageString;

/*!
 * @brief Defines a list of commands that the plugin will support as server input.
 *
 * @return An NSArray containing a lowercase list of commands that the plugin
 *  will support as server input.
 *
 * @discussion If a raw numeric (a number) is being asked for, then insert it into
 *  the array as an NSString.
 */
@property (nonatomic, readonly, copy) NSArray *subscribedServerInputCommands;

/*!
 * @brief Method invoked when a subscribed server input command requires processing.
 *
 * @param inputObject An instance of THOPluginDidReceiveServerInputConcreteObject
 * @param client The client responsible for the event
 *
 * @see THOPluginDidReceiveServerInputConcreteObject
 */
- (void)didReceiveServerInput:(THOPluginDidReceiveServerInputConcreteObject *)inputObject onClient:(IRCClient *)client;

#pragma mark -
#pragma mark Initialization

/*!
 * @brief Method invoked during initialization of a plugin.
 *
 * @discussion This method is invoked very early on. It occurs once the principal 
 *  class of the plugin has been allocated and is guaranteed to be the first call
 *  home that a plugin will receive from Textual.
 */
- (void)pluginLoadedIntoMemory;

/*!
 * @brief Method invoked prior to deallocation of a plugin.
 */
- (void)pluginWillBeUnloadedFromMemory;

#pragma mark -
#pragma mark Preferences Pane

/*!
 * @brief Defines an NSView used by the Preferences window of Textual to
 *  allow user-interactive configuration of the plugin.
 *
 * @discussion Textual 5.1.2 and later use auto layout for the Preferences 
 *  window. Therefore, for a preference pane to be displayed properly,
 *  a width and height constraint must be set on the returned view.
 *
 * @return An instance of NSView with a width of at least 589. This width
 *  is not enforced, but having a view with a width lower than this magic
 *  number will result in one or more toolbar items not fitting on screen.
 */
@property (nonatomic, readonly, strong) NSView *pluginPreferencesPaneView;

/*!
 * @brief Defines an NSString which is used by the Preferences window of
 *  Textual to create a new entry in its navigation list.
 */
@property (nonatomic, readonly, copy) NSString *pluginPreferencesPaneMenuItemName;

#pragma mark -
#pragma mark Renderer Events

/*!
 * @brief Method invoked when the Document Object Model (DOM) of a view has been modified.
 *
 * @discussion This method is invoked when a message has been added to the Document Object 
 *  Model (DOM) of logController
 *
 * @warning It is NOT recommended to do any heavy work when the -isProcessedInBulk property
 *  of the posted message is set to YES because thousand of other messages may be processing
 *  at the same time which can overload the user's Mac if you work on each message.
 *
 * @warning This method is invoked on an asynchronous background dispatch queue. Not the
 *  main thread. It is extremely important to remember this because WebKit will throw an
 *  exception if it is not interacted with on the main thread.
 *
 * @param messageObject An instance of THOPluginDidPostNewMessageConcreteObject
 * @param logController The view responsible for the event
 *
 * @see THOPluginDidPostNewMessageConcreteObject
 */
- (void)didPostNewMessage:(THOPluginDidPostNewMessageConcreteObject *)messageObject forViewController:(TVCLogController *)logController;

#pragma mark -

/*!
 * @brief Method invoked prior to a message being converted to its HTML equivalent.
 *
 * @discussion This gives a plugin the chance to modify the text that will be displayed 
 * for a certain message by replacing one or more segments of it.
 * 
 * Considerations:
 *
 * 1. Returning nil or a string with zero length from this method will indicate that there is
 *  no interest in modifying newMessage.
 *
 * 2. There is no way to inform the renderer that you do not want a specific value of newMessage
 *  shown to the end user. Use the intercept* methods for this purpose.
 *
 * @warning This method is invoked on each plugin in the order loaded. This method does not 
 *  stop for the first result returned which means that value being passed may have been
 *  modified by a plugin above the one being talked to.
 *
 * @warning Under no circumstances should you insert HTML at this point. Doing so will result 
 *  in undefined behavior.
 * 
 * @return The original and/or modified copy of newMessage
 *
 * @param newMessage An unedited copy of the message being rendered
 * @param viewController The view responsible for the event
 * @param lineType The line type of the message being rendered
 * @param memberType The member type of the message being rendered
 */
- (NSString *)willRenderMessage:(NSString *)newMessage
			  forViewController:(TVCLogController *)viewController
					   lineType:(TVCLogLineType)lineType
					 memberType:(TVCLogLineMemberType)memberType;

#pragma mark -

/*!
 * @brief Given a URL, returns the same URL or another that can be shown as an 
 *  image inline with chat.
 *
 * @return A URL that can be shown as an inline image in relation to resource or 
 *  nil to ignore.
 *
 * @discussion Considerations:
 *
 * 1. The return value must be a valid URL for an image file if non-nil.
 *  Textual validates the return value by attempting to create an instance of NSURL
 *  with it. If NSURL returns a nil object, then it is certain that a plugin returned
 *  a bad value.
 *
 * 2. Textual uses the first non-nil, valid URL, returned by a plugin. It does not
 *  chain the responses similar to other methods defined by the THOPluginProtocol
 *  protocol.
 *
 * @param resource A URL that was detected in a message being rendered.
 */
- (NSString *)processInlineMediaContentURL:(NSString *)resource;

#pragma mark -
#pragma mark Input Manipulation

/*!
 * @brief Method invoked to inform the plugin that a plain text message was received 
 *  (PRIVMSG, ACTION, or NOTICE)
 *
 * @discussion This method is invoked on the main thread which means that slow code
 *  can lockup the user interface of Textual. If you have no intent to ignore content,
 *  then do work in the background and immediately return YES when this method is invoked.
 *
 * @return YES to display the contents of the message to the user, NO otherwise.
 *
 * @param text The message contents
 * @param textAuthor The author (sender) of the message
 * @param textDestination The channel that the message is destined for
 * @param lineType The line type of the message. Possible values:
 *          TVCLogLinePrivateMessageType, TVCLogLineActionType, TVCLogLineNoticeType
 * @param client The client the message was received on
 * @param receivedAt The date & time of the message. Depending on whether a custom 
 *          value was specified using the server-time IRCv3 capacity, this NSDate
 *          object may be very far in the past, or even possibly in the future.
 * @param wasEncrypted Whether or not the message was encrypted.
 */
- (BOOL)receivedText:(NSString *)text
          authoredBy:(IRCPrefix *)textAuthor
         destinedFor:(IRCChannel *)textDestination
          asLineType:(TVCLogLineType)lineType
            onClient:(IRCClient *)client
          receivedAt:(NSDate *)receivedAt
        wasEncrypted:(BOOL)wasEncrypted;

/*!
 * @brief Method used to modify and/or completely ignore incoming data from
 *  the server before any action can be taken on it by Textual.
 *
 * @warning This method is invoked on each plugin in the order loaded. This method 
 *  does not stop for the first result returned which means that value being passed may
 *  have been modified by a plugin above the one being talked to.
 *
 * @warning Textual does not perform validation against the instance of IRCMessage that 
 *  is returned which means that if Textual tries to access specific information which has
 *  been improperly modified or removed, the entire application may crash.
 * 
 * @return The original and/or modified copy of IRCMessage or nil to prevent the data from being processed altogether.
 *
 * @param input An instance of IRCMessage which is the container class for parsed incoming data
 * @param client The client responsible for the event
 */
- (IRCMessage *)interceptServerInput:(IRCMessage *)input for:(IRCClient *)client;

/*!
 * @brief Method used to modify and/or completely ignore text entered into the
 *  main text field of Textual by the end user.
 *
 * @discussion This method is invoked once the user has hit return on the text field 
 *  to submit whatever its value may be.
 *
 * @warning This method is invoked on each plugin in the order loaded. This method
 *  does not stop for the first result returned which means that value being passed may
 *  have been modified by a plugin above the one being talked to.
 * 
 * @return The original and/or modified copy of input or nil to prevent the data from 
 *  being processed altogether.
 * 
 * @param input Depending on whether the value of the text field was submitted 
 *  programmatically or by the user directly interacting with it, this value can be an
 *  instance of NSString or NSAttributedString.
 * @param command Textual allows the end user to send text entered into the text field 
 *  without using the "/me" command. When this occurs, Textual informs lower-level APIs
 *  of this intent by changing the value of this parameter from "privmsg" to "action" -
 *  In most cases a plugin should disregard this parameter and pass it untouched.
 */
- (id)interceptUserInput:(id)input command:(NSString *)command;

#pragma mark -
#pragma mark Reserved Calls

/* The behavior of this method call is undefined. It exists for internal
 purposes for the plugins packaged with Textual by default. It is not
 recommended to use it, or try to understand it. */
@property (nonatomic, readonly, copy) NSArray *pluginOutputSuppressionRules;

#pragma mark -
#pragma mark Deprecated

- (void)didReceiveServerInputOnClient:(IRCClient *)client senderInformation:(NSDictionary *)senderDict messageInformation:(NSDictionary *)messageDict TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");

TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputSenderIsServerAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputSenderHostmaskAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputSenderNicknameAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputSenderUsernameAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputSenderAddressAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputMessageReceivedAtTimeAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputMessageParamatersAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputMessageCommandAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputMessageNumericReplyAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputMessageSequenceAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputMessageNetworkAddressAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidReceiveServerInputMessageNetworkNameAttribute TEXTUAL_DEPRECATED("Use -didReceiveServerInput:onClient: instead");

- (void)didPostNewMessageForViewController:(TVCLogController *)logController messageInfo:(NSDictionary *)messageInfo isThemeReload:(BOOL)isThemeReload isHistoryReload:(BOOL)isHistoryReload TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");

TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageLineNumberAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageSenderNicknameAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageLineTypeAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageMemberTypeAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageReceivedAtTimeAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageListOfHyperlinksAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageListOfUsersAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageMessageBodyAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
TEXTUAL_EXTERN NSString * const THOPluginProtocolDidPostNewMessageKeywordMatchFoundAttribute TEXTUAL_DEPRECATED("Use -didPostNewMessage:forViewController: instead");
@end

#pragma mark -

@interface THOPluginDidPostNewMessageConcreteObject : NSObject
/*!
 * @brief Specifies whether the message was posted as a result of a bulk operation.
 */
@property (readonly) BOOL isProcessedInBulk;

/*!
 * @brief The contents of the message visible to the end user, minus any formatting.
 */
@property (readonly, copy) NSString *messageContents;

/*!
 * @brief The ID of the message that can be used to access it using getElementByID()
 */
@property (readonly, copy) NSString *lineNumber;

/*!
 * @brief The nickname of the person and/or server responsible for producing the 
 *  message. This value may be empty. Not every event on IRC will have a sender value.
 */
@property (readonly, copy) NSString *senderNickname;

/*!
 * @brief The line type of the message that was posted.
 */
@property (readonly) TVCLogLineType lineType;

/*!
 * @brief The member type of the message that was posted. 
 */
@property (readonly) TVCLogLineMemberType memberType;

/*!
 * @brief The date & time displayed left of the message in the chat view.
 */
@property (readonly, copy) NSDate *receivedAt;

/*!
 * @brief Array of ranges (NSRange) of text in the message body believed to be a URL.
 *
 * @discussion Each entry in this array is another array containing two indexes. First
 *  index (0) is the range in -messageContents that the URL was at. The second index (1)
 *  is the URL that was found. The URL may differ from the value in the range as URL 
 *  schemes may have been appended. For example, the text at the given range may be 
 *  "www.example.com" whereas the entry at index 1 is "http://www.example.com"
 */
@property (readonly, copy) NSArray *listOfHyperlinks;

/*!
 * @brief List of users from the channel that appear in the message body.
 */
@property (readonly, copy) NSSet *listOfUsers;

/*!
 * @brief Whether or not a highlight word was matched in the message body.
 */
@property (readonly) BOOL keywordMatchFound;
@end

#pragma mark -

@interface THOPluginDidReceiveServerInputConcreteObject : NSObject
/*!
 * @brief Whether the input was from a regular user or from a server.
 */
@property (readonly) BOOL senderIsServer;

/*!
 * @brief The nickname portion of the sender's hostmask.
 *
 * @discussion The value of this property is the server address if isServer == YES
 */
@property (readonly, copy) NSString *senderNickname;

/*!
 * @brief The username (ident) portion of the sender's hostmask.
 */
@property (readonly, copy) NSString *senderUsername;

/*!
 * @brief The address portion of the sender's hostmask.
 */
@property (readonly, copy) NSString *senderAddress;

/*!
 * @brief The combined hostmask of the sender.
 */
@property (readonly, copy) NSString *senderHostmask;

/*!
 * @brief The date & time during which the input was received.
 *
 * @discussion If the original message specifies a custom value using the server-time
 *  capacity, then the value of this property will reflect the value defined by the
 *  server-time capacity; not the exact date & time it was received on the socket.
 */
@property (readonly, copy) NSDate *receivedAt;

/*!
 * @brief The input itself
 */
@property (readonly, copy) NSString *messageSequence;

/*!
 * @brief The input, split up into sections
 */
@property (readonly, copy) NSArray *messageParamaters;

/*!
 * @brief The input's command
 */
@property (readonly, copy) NSString *messageCommand;

/*!
 * @brief The value of -messageCommand as an integer
 */
@property (readonly) NSInteger messageCommandNumeric;

/*!
 * @brief The server address of the IRC network
 *
 * @discussion The value of this attribute is the address of the server that 
 *  Textual is currently connected to and may differ from -senderNickanme
 */
@property (readonly, copy) NSString *networkAddress;

/*!
 * @brief The name of the IRC network
 */
@property (readonly, copy) NSString *networkName;
@end

#pragma mark -

@interface THOPluginOutputSuppressionRule : NSObject
@property (nonatomic, copy) NSString *match;
@property (nonatomic, assign) BOOL restrictConsole;
@property (nonatomic, assign) BOOL restrictChannel;
@property (nonatomic, assign) BOOL restrictPrivateMessage;
@end

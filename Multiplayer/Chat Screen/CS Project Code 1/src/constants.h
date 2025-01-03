#ifndef CONSTANTS_H
#define CONSTANTS_H

#define FPS 200

constexpr size_t MAX_USERNAME_LENGTH = 10;
constexpr size_t MAX_MESSAGE_LENGTH = 200;
constexpr size_t MAX_MESSAGE_HISTORY_STORAGE_SIZE = 5;

enum ChatEvents
{
    doNothing,
    openClientJoinMenu,
    openHostSettingsMenu
};

enum CurrentMenu
{
    mainMenu,
    clientJoinMenu,
    hostSettingsMenu,
    hostMain,
    clientMain
};

enum MainMenuStates
{
    MMS_HOST,
    MMS_JOIN
};

enum ClientJoinMenuStates
{

};

enum HostSettingsMenuState
{

};

enum HostMainState
{

};

enum ClientMainState
{

};



#endif
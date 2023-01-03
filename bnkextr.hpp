#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <cstdint>
#include <map>

#define ALIGN(value, align) value + ((align - (value % align)) % align)

struct Index;
struct Section;

#pragma pack(push, 1)
struct Index
{
    std::uint32_t id;
    std::uint32_t offset;
    std::uint32_t size;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Section
{
    char sign[4];
    std::uint32_t size;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BankHeader
{
    std::uint32_t version;
    std::uint32_t id;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PCK_Header
{
    std::uint32_t unkn2;
    std::uint32_t languageLength;
    std::uint32_t bnkTableLength;
    std::uint32_t wemTableLength;
    std::uint32_t unkn6;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct stringData
{
    std::uint32_t postHeaderOffset;
    std::uint32_t index;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct embeddedMedia
{
    uint32_t ID;
    uint32_t unkn;
    uint32_t Length;
    uint32_t Offset;
    uint32_t language;
};
#pragma pack(pop)

enum class ObjectType : std::int8_t
{
    Settings = 1,
    Sound = 2,
    EventAction = 3,
    Event = 4,
    RandomOrSequenceContainer = 5,
    SwitchContainer = 6,
    ActorMixer = 7,
    AudioBus = 8,
    BlendContainer = 9,
    MusicSegment = 10,
    MusicTrack = 11,
    MusicSwitchContainer = 12,
    MusicPlaylistContainer = 13,
    Attenuation = 14,
    DialogueEvent = 15,
    MotionBus = 16,
    MotionFx = 17,
    Effect = 18,
    Unknown = 19,
    AuxiliaryBus = 20
};

std::string GetObjectTypeName(ObjectType Object)
{
    switch (Object)
    {
    case ObjectType::Settings:
        return "Settings";
        break;

    case ObjectType::Sound:
        return "Sound";
        break;

    case ObjectType::EventAction:
        return "EventAction";
        break;

    case ObjectType::Event:
        return"Event";
        break;

    case ObjectType::RandomOrSequenceContainer:
        return "RandomOrSequenceContainer";
        break;

    case ObjectType::SwitchContainer:
        return "SwitchContainer";
        break;

    case ObjectType::ActorMixer:
        return "ActorMixer";
        break;

    case ObjectType::AudioBus:
        return "AudioBus";
        break;

    case ObjectType::BlendContainer:
        return "BlendContainer";
        break;

    case ObjectType::MusicSegment:
        return "MusicSegment";
        break;

    case ObjectType::MusicTrack:
        return "MusicTrack";
        break;

    case ObjectType::MusicSwitchContainer:
        return "MusicSwitchContainer";
        break;

    case ObjectType::MusicPlaylistContainer:
        return "MusicPlaylistContainer";
        break;

    case ObjectType::Attenuation:
        return "Attenuation";
        break;

    case ObjectType::DialogueEvent:
        return "DialogueEvent";
        break;

    case ObjectType::MotionBus:
        return "MotionBus";
        break;

    case ObjectType::MotionFx:
        return "MotionFx";
        break;

    case ObjectType::Effect:
        return "Effect";
        break;

    case ObjectType::Unknown:
        return "Unknown";
        break;

    case ObjectType::AuxiliaryBus:
        return "AuxiliaryBus";
        break;

    default:
        return "Unknown";
        break;
    };
}

#pragma pack(push, 1)
struct Object
{
    ObjectType type;
    std::uint32_t size;
    std::uint32_t id;
};
#pragma pack(pop)

struct EventObject
{
    std::uint32_t action_count;
    std::vector<std::uint32_t> action_ids;
};

enum class EventActionScope : std::int8_t
{
    SwitchOrTrigger = 1,
    Global = 2,
    GameObject = 3,
    State = 4,
    All = 5,
    AllExcept = 6
};

std::string GetEventActionName(EventActionScope Event)
{
    switch (Event)
    {
    case EventActionScope::SwitchOrTrigger:
        return "SwitchOrTrigger";
        break;

    case EventActionScope::Global:
        return "Global";
        break;

    case EventActionScope::GameObject:
        return "GameObject";
        break;

    case EventActionScope::State:
        return "State";
        break;

    case EventActionScope::All:
        return "All";
        break;

    case EventActionScope::AllExcept:
        return "AllExcept";
        break;

    default:
        return "Unknown";
        break;
    }
}

enum class EventActionType : std::int8_t
{
    Stop = 1,
    Pause = 2,
    Resume = 3,
    Play = 4,
    Trigger = 5,
    Mute = 6,
    UnMute = 7,
    SetVoicePitch = 8,
    ResetVoicePitch = 9,
    SetVoiceVolume = 10,
    ResetVoiceVolume = 11,
    SetBusVolume = 12,
    ResetBusVolume = 13,
    SetVoiceLowPassFilter = 14,
    ResetVoiceLowPassFilter = 15,
    EnableState = 16,
    DisableState = 17,
    SetState = 18,
    SetGameParameter = 19,
    ResetGameParameter = 20,
    SetSwitch = 21,
    ToggleBypass = 22,
    ResetBypassEffect = 23,
    Break = 24,
    Seek = 25
};

std::string GetActionName(EventActionType Action)
{
    switch (Action)
    {
    case EventActionType::Stop:
        return "Stop";
        break;

    case EventActionType::Pause:
        return "Pause";
        break;

    case EventActionType::Resume:
        return "Resume";
        break;

    case EventActionType::Play:
        return "Play";
        break;

    case EventActionType::Trigger:
        return "Trigger";
        break;

    case EventActionType::Mute:
        return "Mute";
        break;

    case EventActionType::UnMute:
        return "UnMute";
        break;

    case EventActionType::SetVoicePitch:
        return "SetVoicePitch";
        break;

    case EventActionType::ResetVoicePitch:
        return "ResetVoicePitch";
        break;

    case EventActionType::SetVoiceVolume:
        return "SetVoiceVolume";
        break;

    case EventActionType::ResetVoiceVolume:
        return "ResetVoiceVolume";
        break;

    case EventActionType::SetBusVolume:
        return "SetBusVolume";
        break;

    case EventActionType::ResetBusVolume:
        return "ResetBusVolume";
        break;

    case EventActionType::SetVoiceLowPassFilter:
        return "SetVoiceLowPassFilter";
        break;

    case EventActionType::ResetVoiceLowPassFilter:
        return "ResetVoiceLowPassFilter";
        break;

    case EventActionType::EnableState:
        return "EnableState";
        break;

    case EventActionType::DisableState:
        return "DisableState";
        break;

    case EventActionType::SetState:
        return "SetState";
        break;

    case EventActionType::SetGameParameter:
        return "SetGameParameter";
        break;

    case EventActionType::ResetGameParameter:
        return "ResetGameParameter";
        break;

    case EventActionType::SetSwitch:
        return "SetSwitch";
        break;

    case EventActionType::ToggleBypass:
        return "ToggleBypass";
        break;

    case EventActionType::ResetBypassEffect:
        return "ResetBypassEffect";
        break;

    case EventActionType::Break:
        return "Break";
        break;

    case EventActionType::Seek:
        return "Seek";
        break;

    default:
        return "Unknown";
        break;
    }
}

enum class EventActionParameterType : std::int8_t
{
    Delay = 0x0E,
    Play = 0x0F,
    Probability = 0x10
};

std::string GetEventActionParameterName(EventActionParameterType Action)
{
    switch (Action)
    {
    case EventActionParameterType::Delay:
        return "Delay";
        break;

    case EventActionParameterType::Play:
        return "Play";
        break;

    case EventActionParameterType::Probability:
        return "Probability";
        break;

    default:
        return "Unknown";
        break;
    }
}

enum class StorageType : std::int8_t
{
    Embedded = 0,
    Streamed = 1,
    StreamedPrefetched = 2
};

std::string GetStorageTypeName(StorageType Type)
{
    switch (Type)
    {
    case StorageType::Embedded:
        return "Embedded";
        break;

    case StorageType::Streamed:
        return "Streamed";
        break;

    case StorageType::StreamedPrefetched:
        return "StreamedPrefetched";
        break;

    default:
        return "Unknown";
        break;
    }
}

enum class SoundType : std::int8_t
{
    SFX = 0,
    Voice = 1
};

std::string GetSoundTypeName(SoundType Type)
{
    switch (Type)
    {
    case SoundType::SFX:
        return "SFX";
        break;

    case SoundType::Voice:
        return "Voice";
        break;

    default:
        return "Unknown";
        break;
    }
}

struct EventActionObject
{
    EventActionScope scope;
    EventActionType action_type;
    std::uint32_t game_object_id;
    std::uint8_t parameter_count;
    std::vector<EventActionParameterType> parameters_types;
    std::vector<std::int8_t> parameters;
};

struct Sound
{
    std::uint32_t PluginID;
    StorageType state;
    std::uint32_t AudioID;
    std::uint32_t SourceID;
    SoundType SoundType;
    std::uint32_t InMemoryMediaSize;
};
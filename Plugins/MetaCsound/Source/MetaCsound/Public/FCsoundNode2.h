#pragma once

#include "TCsoundOperator.h"
#include "MetasoundExecutableOperator.h"     // TExecutableOperator class
#include "MetasoundPrimitives.h"             // ReadRef and WriteRef descriptions for bool, int32, float, and string
#include "MetasoundNodeRegistrationMacro.h"  // METASOUND_LOCTEXT and METASOUND_REGISTER_NODE macros
#include "MetasoundFacade.h"				 // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "Containers/Array.h"

#define LOCTEXT_NAMESPACE "MetaCsound_FCsoundNode2"

namespace MetaCsound
{

    class METACSOUND_API FCsoundOperator2 : public TCsoundOperator<FCsoundOperator2>
    {
    public:
        // TODO: create struct that has all of the arguments, to avoid such long constructors?
        FCsoundOperator2(const FOperatorSettings& InSettings,
            const FTriggerReadRef& InPlayTrigger,
            const FTriggerReadRef& InStopTrigger,
            const FStringReadRef& InCsoundFile,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const int32& InNumOutAudioChannels,
            const TArray<FFloatReadRef>& InControlRefs,
            const int32& InNumOutControlChannels
        )
            : TCsoundOperator(
                InSettings, InPlayTrigger, InStopTrigger, InCsoundFile,
                InAudioRefs, InNumOutAudioChannels, InControlRefs, InNumOutControlChannels
            )
        { }

        // TODO: inline?
        static const FNodeClassName GetClassName()
        {
            // WIP What is Audio? Could we use "Csound" instead?
            return { TEXT("MetaCsound"), TEXT("Csound2"), TEXT("Audio") };
        }

        static const FText GetDisplayName()
        {
            return LOCTEXT("NodeDisplayName", "Csound (2)");
        }

        static const FText GetDescription()
        {
            return LOCTEXT("NodeDesc", "Csound with stereo input and output");
        }

        static constexpr int32 NumAudioChannelsIn = 2;
        static constexpr int32 NumAudioChannelsOut = 2;
        static constexpr int32 NumControlChannelsIn = 8;
        static constexpr int32 NumControlChannelsOut = 4;
    };

    class METACSOUND_API FCsoundNode2 : public FNodeFacade
    {
    public:
        FCsoundNode2(const FNodeInitData& InitData) : FNodeFacade(InitData.InstanceName, InitData.InstanceID,
            TFacadeOperatorClass<FCsoundOperator2>())
        { }
    };

    // Register node
    METASOUND_REGISTER_NODE(FCsoundNode2); // WIP Node registration using module startup/shutdown?

}

#undef LOCTEXT_NAMESPACE

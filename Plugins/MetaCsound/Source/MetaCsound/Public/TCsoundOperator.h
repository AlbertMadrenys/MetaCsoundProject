#pragma once

// Is this still necessary? WIP
#ifdef LIKELY
    #undef LIKELY
#endif

#ifdef UNLIKELY
    #undef UNLIKELY
#endif

THIRD_PARTY_INCLUDES_START
#include <csound.hpp>
THIRD_PARTY_INCLUDES_END

#include "MetasoundExecutableOperator.h"     // TExecutableOperator class
#include "MetasoundPrimitives.h"             // ReadRef and WriteRef descriptions for bool, int32, float, and string
#include "MetasoundNodeRegistrationMacro.h"  // METASOUND_LOCTEXT and METASOUND_REGISTER_NODE macros
#include "MetasoundFacade.h"				 // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "Containers/Array.h"

// WIP trying to create my own pin type
#include "MetasoundDataTypeRegistrationMacro.h"
#include "MetasoundDataReferenceMacro.h"
#include "MetasoundDataReference.h"
#include "MetasoundVariable.h"

#include "MetaCsound.h"

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetaCsound_CsoundNode"

// WIP: MetaCsound should be a nested namespace with UE::Audio:: or Metasound:: ?
namespace MetaCsound
{
    using namespace Metasound;

    template<typename DerivedOperator>
    class TCsoundOperator : public TExecutableOperator<DerivedOperator>
    {

    protected:
        // Protected constructor
        TCsoundOperator(const FOperatorSettings& InSettings,
            const FTriggerReadRef& InPlayTrigger,
            const FTriggerReadRef& InStopTrigger,
            const FStringReadRef& InFilePath,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const int32& InNumOutAudioChannels,
            const TArray<FFloatReadRef>& InControlRefs,
            const int32& InNumOutControlChannels,
            const FStringReadRef& InEventString,
            const FTriggerReadRef& InEventTrigger
        );

    public:

        // Primary node functionality
        void Execute();

        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& DeclareVertexInterface();

        // Allows MetaSound graph to interact with your node's inputs
        virtual void BindInputs(Metasound::FInputVertexInterfaceData& InOutVertexData) override final;
        // Allows MetaSound graph to interact with your node's outputs
        virtual void BindOutputs(Metasound::FOutputVertexInterfaceData& InOutVertexData) override final;
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors);

    private:

        FTriggerReadRef PlayTrigger;
        FTriggerReadRef StopTrigger;
        FStringReadRef FilePath;
        FStringReadRef EventString;
        FTriggerReadRef EventTrigger;
        FTriggerWriteRef FinishedTrigger;

        TArray<FAudioBufferReadRef> AudioInRefs;
        TArray<const float*> BuffersIn;
        TArray<FAudioBufferWriteRef> AudioOutRefs;
        TArray<float*> BuffersOut;

        TArray<FFloatReadRef> ControlInRefs;
        TArray<FString> ControlInNames;
        TArray<FFloatWriteRef> ControlOutRefs;
        TArray<FString> ControlOutNames;

        FOperatorSettings OpSettings;
        Csound CsoundInstance;
        int32 SpIndex;

        double* Spin, * Spout;
        int32 CsoundNchnlsIn, CsoundNchnlsOut, MinAudioIn, MinAudioOut;
        int32 CsoundKsmps;
        int32 FirstClearedFrame;

        enum class EOpState : uint8
        {
            Stopped,
            Playing,
            Error
        };

        EOpState OpState;

        void Play(int32 CurrentFrame);
        void Stop(int32 StopFrame = 0);
        void ClearChannels(int32 StartClearingFrame = 0);
        void CsoundPerformKsmps(int32 CurrentFrame);
    };
    
    class METACSOUND_API FCsoundOperator2 : public TCsoundOperator<FCsoundOperator2>
    {
    public:
        // WIP create struct that has all of the arguments, to avoid such long constructors?
        FCsoundOperator2(const FOperatorSettings& InSettings,
            const FTriggerReadRef& InPlayTrigger,
            const FTriggerReadRef& InStopTrigger,
            const FStringReadRef& InFilePath,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const int32& InNumOutAudioChannels,
            const TArray<FFloatReadRef>& InControlRefs,
            const int32& InNumOutControlChannels,
            const FStringReadRef& InEventString,
            const FTriggerReadRef& InEventTrigger
        )
        : TCsoundOperator(
            InSettings, InPlayTrigger, InStopTrigger, InFilePath,
            InAudioRefs, InNumOutAudioChannels, InControlRefs, InNumOutControlChannels,
            InEventString, InEventTrigger
        )
        { }

        // WIP inline?
        static const FNodeClassName GetClassName() 
        {
            // WIP What is Audio? Could we use "Csound" instead?
            return { TEXT("MetaCsound"), TEXT("Csound 2"), TEXT("Audio") }; 
        }

        static const FText GetDisplayName()
        {
            return LOCTEXT("MetaCsound_Node2DisplayName", "Csound 2");
        }

        static const FText GetDescription()
        {
            return LOCTEXT("MetaCsound_Node2Desc", "Csound 2 description");
        }

        static constexpr int32 NumAudioChannelsIn = 2;
        static constexpr int32 NumAudioChannelsOut = 2;
        static constexpr int32 NumControlChannelsIn = 2;
        static constexpr int32 NumControlChannelsOut = 2;
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

    class METACSOUND_API FCsoundOperator4 : public TCsoundOperator<FCsoundOperator4>
    {
    public:
        
        FCsoundOperator4(const FOperatorSettings& InSettings,
            const FTriggerReadRef& InPlayTrigger,
            const FTriggerReadRef& InStopTrigger,
            const FStringReadRef& InFilePath,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const int32& InNumOutAudioChannels,
            const TArray<FFloatReadRef>& InControlRefs,
            const int32& InNumOutControlChannels,
            const FStringReadRef& InEventString,
            const FTriggerReadRef& InEventTrigger
        )
        : TCsoundOperator(
            InSettings, InPlayTrigger, InStopTrigger, InFilePath,
            InAudioRefs, InNumOutAudioChannels, InControlRefs, InNumOutControlChannels,
            InEventString, InEventTrigger
        )
        { }

        // WIP inline?
        static const FNodeClassName GetClassName()
        {
            // WIP What is Audio? Could we change it to something like "Csound" instead?
            return { TEXT("MetaCsound"), TEXT("Csound 4"), TEXT("Audio") };
        }

        static const FText GetDisplayName()
        {
            return LOCTEXT("MetaCsound_Node2DisplayName", "Csound 4");
        }

        static const FText GetDescription()
        {
            return LOCTEXT("MetaCsound_Node2Desc", "Csound 4 description");
        }

        static constexpr int32 NumAudioChannelsIn = 4;
        static constexpr int32 NumAudioChannelsOut = 4;
        static constexpr int32 NumControlChannelsIn = 4;
        static constexpr int32 NumControlChannelsOut = 4;
    };

    class METACSOUND_API FCsoundNode4 : public FNodeFacade
    {
    public:
        FCsoundNode4(const FNodeInitData& InitData) : FNodeFacade(InitData.InstanceName, InitData.InstanceID,
            TFacadeOperatorClass<FCsoundOperator4>())
        { }
    };

    // Register node
    METASOUND_REGISTER_NODE(FCsoundNode4);
    
}

#undef LOCTEXT_NAMESPACE

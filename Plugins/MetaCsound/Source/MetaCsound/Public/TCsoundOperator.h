#pragma once

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

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetaCsound_TCsoundOperator"

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
            const FStringReadRef& InCsoundFile,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const int32& InNumOutAudioChannels,
            const TArray<FFloatReadRef>& InControlRefs,
            const int32& InNumOutControlChannels
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
        FStringReadRef CsoundFile;
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
    
}

#undef LOCTEXT_NAMESPACE

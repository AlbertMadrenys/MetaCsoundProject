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
#include "MetasoundStandardNodesNames.h"     // StandardNodes namespace
#include "MetasoundFacade.h"				 // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros
//#include "MetasoundNodeInterface.h"
#include "Containers/Array.h"
//#include "Containers/StaticArray.h"

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetaCsound_CsoundNode"

/*
namespace Metasound
{
    DECLARE_METASOUND_DATA_REFERENCE_TYPES(UObject, METASOUNDFRONTEND_API, FUObjectTypeInfo, FUObjectReadRef, FUObjectWriteRef);
}*/

// WIP: Should I use namespace Metasound?
namespace MetaCsound
{
    using namespace Metasound;

    METASOUND_PARAM(CsoundFilePath, "File", "Path of the .csd file to be executed by Csound");
    METASOUND_PARAM(EventString, "Event String", "The string that contains a Csound event");
    METASOUND_PARAM(EventTrigger, "Event Trigger", "Triggers the Csound event descrived by EventString");

    METASOUND_PARAM(InputAudio1, "InAudio1", "Input audio 1");
    METASOUND_PARAM(InputAudio2, "InAudio2", "Input audio 2");

    // Global constant named InputAudio1##Name

    METASOUND_PARAM(OutputAudio1, "OutAudio1", "Output audio 1");
    METASOUND_PARAM(OutputAudio2, "OutAudio2", "Output audio 2");

    METASOUND_PARAM(InputControl1, "InControl1", "Input control 1");

    class FCsoundOperator : public TExecutableOperator<FCsoundOperator>
    {
    public:
        // Constructor
        FCsoundOperator(const FOperatorSettings& InSettings,
            const FStringReadRef& FilePath,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const size_t& NumOutAudioChannels, // WIP Should I use int8 or size_t?
            const TArray<FFloatReadRef>& InControlRefs,
            const FStringReadRef& EventString,
            const FTriggerReadRef& EventTrigger
            )
            : m_FilePath(FilePath)
            , m_EventString(EventString)
            , m_EventTrigger(EventTrigger)
            , m_InAudioRefs(InAudioRefs)
            , m_InBuffers()
            , m_InControlRefs(InControlRefs)
            , m_OutAudioRefs()
            , m_OutBuffers()
            , m_OpSettings(InSettings)
            , m_CsoundInstance()
            , m_SpIndex(0)
            , m_Spin()
            , m_Spout()
            
        {
            const char* csdFilePath = StringCast<ANSICHAR>(*(*m_FilePath.Get())).Get();
            FString srOptionFString = "--sample-rate=" + FString::FromInt((int)m_OpSettings.GetSampleRate());
            const char* srOption = StringCast<ANSICHAR>(*srOptionFString).Get();

            // WIP make not explode it when compilation error
            m_CsoundInstance.Compile(csdFilePath, srOption, "-n");

            m_Spin = m_CsoundInstance.GetSpin();
            m_Spout = m_CsoundInstance.GetSpout();
            
            // WIP Make schedule "f0 z" so it runs indefinetely independently of the score, it's not working right now
            // Csound Unity does not have this option either, the user has to add it manually to the csd file
            // --daemon to do that
            m_CsoundInstance.InputMessage("f0 z");

            // WIP Create a stop event?
            // WIP when Csound stops, an annoying sound gets outputed, stop that
            
            GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,
                FString::Printf(TEXT("sr: %f,csound sr: %f")
                    , m_OpSettings.GetSampleRate()
                    , m_CsoundInstance.GetSr()));

            m_InBuffers.Empty(2);
            for (size_t i = 0; i < 2; i++)
            {
                m_InBuffers.Add(m_InAudioRefs[i]->GetData());
            }

            m_OutAudioRefs.Empty(NumOutAudioChannels); // WIP Empty vs Reset vs Resize
            m_OutBuffers.Empty(NumOutAudioChannels); // WIP Empty vs Reset vs Resize vs SetNum and []
            for (size_t i = 0; i < NumOutAudioChannels; i++)
            {
                m_OutAudioRefs.Add(FAudioBufferWriteRef::CreateNew(m_OpSettings));
                m_OutBuffers.Add(m_OutAudioRefs[i]->GetData());
            }
            
        };

        // Primary node functionality
        void Execute()
        {
            

            if (m_EventTrigger->IsTriggeredInBlock())
            {
                // WIP Does Csound work in sample accurate score line events? Use a string queue, handlede by anohter node
                // Or the best way would be to use callbacks, does m_EventTrigger allow for callbacks??
                const char* eventCString = StringCast<ANSICHAR>(*(*m_EventString.Get())).Get();
                m_CsoundInstance.InputMessage(eventCString);
            }

            // WIP Should I keep this in a class attribute?
            size_t csoundNchnlsIn = m_CsoundInstance.GetNchnlsInput();
            size_t csoundNchnlsOut = m_CsoundInstance.GetNchnls();
            size_t minIn = m_InBuffers.Num() <= csoundNchnlsIn ? m_InBuffers.Num() : csoundNchnlsIn;
            size_t minOut = m_OutBuffers.Num() <= csoundNchnlsOut ? m_OutBuffers.Num() : csoundNchnlsOut;
            uint32 csoundKsmps = (uint32)m_CsoundInstance.GetKsmps();

            for (size_t f = 0; f < m_OpSettings.GetNumFramesPerBlock(); f++)
            {
                for (size_t i = 0; i < minIn; i++)
                {
                    // WIP Use version of Csound that uses floats instead of doubles?
                    m_Spin[m_SpIndex * csoundNchnlsIn + i] = (double)(m_InBuffers[i][f]);
                }
                
                for (size_t i = 0; i < minOut; i++)
                {
                    m_OutBuffers[i][f] = (float)m_Spout[m_SpIndex * csoundNchnlsOut + i];
                }

                if (++m_SpIndex >= csoundKsmps)
                {

                    // Update multiple in control channels WIP
                    m_CsoundInstance.SetControlChannel("amp", (double)*(m_InControlRefs[0]));

                    m_CsoundInstance.PerformKsmps();
                    m_SpIndex = 0;

                    // OnFinished event trigger
                    // csound.destroy and csound.compile to recompile to make it like a loop

                    // Update multiple out control channels WIP
                }
            }
        }

        static const FNodeClassMetadata& GetNodeInfo()
        {
            auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
                {
                    FNodeClassMetadata Info;

                    Info.ClassName = { TEXT("UE"), TEXT("Csound"), TEXT("Audio") };
                    Info.MajorVersion = 1;
                    Info.MinorVersion = 0;
                    Info.DisplayName = LOCTEXT("Metasound_CsoundNodeDisplayName", "Csound");
                    Info.Description = LOCTEXT("Metasound_CsoundNodeDesc", "Csound description");
                    Info.Author = PluginAuthor;
                    Info.PromptIfMissing = PluginNodeMissingPrompt;
                    Info.DefaultInterface = DeclareVertexInterface();
                    Info.CategoryHierarchy = { LOCTEXT("Metasound_CsoundNodeCategory", "Utils") };

                    return Info;
                };

            static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
            return Metadata;
        }

        static const FVertexInterface& DeclareVertexInterface()
        {
            // WIP Could I make this MACROS be dynamic with a For?
            static const FVertexInterface Interface(
                FInputVertexInterface(
                    TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(CsoundFilePath)),
                    TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputAudio1)),
                    TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputAudio2)),
                    TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputControl1)),
                    TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(EventString)),
                    TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(EventTrigger))
                ),
                FOutputVertexInterface(
                    TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputAudio1)),
                    TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputAudio2))
                )
            );

            return Interface;
        }

        // Allows MetaSound graph to interact with your node's inputs
        virtual FDataReferenceCollection GetInputs() const override
        {
            FDataReferenceCollection InputDataReferences;

            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(CsoundFilePath), m_FilePath);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InputAudio1), m_InAudioRefs[0]);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InputAudio2), m_InAudioRefs[1]);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InputControl1), m_InControlRefs[0]);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(EventString), m_EventString);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(EventTrigger), m_EventTrigger);
            /*
            for (int i = 0; i < m_InAudioRefs.Num(); i++)
            {
                InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InputAudio1), m_InAudioRefs[i]);
            }*/

            return InputDataReferences;
        }

        // Allows MetaSound graph to interact with your node's outputs
        virtual FDataReferenceCollection GetOutputs() const override
        {
            FDataReferenceCollection OutputDataReferences;

            OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutputAudio1), m_OutAudioRefs[0]);
            OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutputAudio2), m_OutAudioRefs[1]);

            return OutputDataReferences;
        }

        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors)
        {
            // WIP Separate Vertex Interface features from the dsp core with different files and classes.
            // Different classes that inheritates from a base dsp class with different vertex interface
            const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
            const FInputVertexInterface& InputInterface = DeclareVertexInterface().GetInputInterface();

            TDataReadReference<FString> CsoundFP = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface,
                METASOUND_GET_PARAM_NAME(CsoundFilePath), InParams.OperatorSettings);
            TDataReadReference<FAudioBuffer> InputAudio1 = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FAudioBuffer>(InputInterface,
                METASOUND_GET_PARAM_NAME(InputAudio1), InParams.OperatorSettings);
            TDataReadReference<FAudioBuffer> InputAudio2 = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FAudioBuffer>(InputInterface,
                METASOUND_GET_PARAM_NAME(InputAudio2), InParams.OperatorSettings);
            TDataReadReference<FString> EvString = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface,
                METASOUND_GET_PARAM_NAME(EventString), InParams.OperatorSettings);
            TDataReadReference<FTrigger> EvTrigger = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FTrigger>(InputInterface,
                METASOUND_GET_PARAM_NAME(EventTrigger), InParams.OperatorSettings);
            TDataReadReference<float> InputControl1 = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface,
                METASOUND_GET_PARAM_NAME(InputControl1), InParams.OperatorSettings);

            // WIP Make this array only reserve for the number that we want
            TArray<TDataReadReference<FAudioBuffer>> arrayAudio = { InputAudio1, InputAudio2 };
            //TStaticArray<TDataReadReference<FAudioBuffer>, 2> stat(InputAudio);

            TArray<TDataReadReference<float>> arrayControl = { InputControl1 };

            return MakeUnique<FCsoundOperator>(InParams.OperatorSettings, CsoundFP, arrayAudio, (size_t)2, arrayControl, EvString, EvTrigger);
        }

    private:
        // Inputs
        FStringReadRef m_FilePath;
        FStringReadRef m_EventString;
        FTriggerReadRef m_EventTrigger;

        TArray<FAudioBufferReadRef> m_InAudioRefs;
        TArray<const float*> m_InBuffers;

        TArray<FFloatReadRef> m_InControlRefs;
        
        // Outputs
        TArray<FAudioBufferWriteRef> m_OutAudioRefs;
        TArray<float*> m_OutBuffers;

        FOperatorSettings m_OpSettings;
        Csound m_CsoundInstance;
        uint32 m_SpIndex;
        double *m_Spin, *m_Spout;
    };

    class FCsoundNode : public FNodeFacade
    {
    public:
        FCsoundNode(const FNodeInitData& InitData) : FNodeFacade(InitData.InstanceName, InitData.InstanceID,
            TFacadeOperatorClass<FCsoundOperator>())
        { }
    };

    // Register node
    METASOUND_REGISTER_NODE(FCsoundNode);
}

#undef LOCTEXT_NAMESPACE
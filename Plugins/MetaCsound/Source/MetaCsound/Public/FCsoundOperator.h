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
#include "MetasoundStandardNodesNames.h"     // StandardNodes namespace
#include "MetasoundFacade.h"				 // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros
//#include "MetasoundNodeInterface.h"
#include "Containers/Array.h"
//#include "Containers/StaticArray.h"
//#include "GenericPlatform/StandardPlatformString.h"

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

    //static const TArray<TCHAR*> hi = { TEXT("InControl_1"), TEXT("InControl_2"),
    //    TEXT("InControl_1"), TEXT("InControl_1") };

    METASOUND_PARAM(CsoundFilePath, "File", "Path of the .csd file to be executed by Csound");
    METASOUND_PARAM(EventString, "Event String", "The string that contains a Csound event");
    METASOUND_PARAM(EventTrigger, "Event Trigger", "Triggers the Csound event descrived by EventString");

    METASOUND_PARAM(InA, "In Audio {0}", "Input audio {0}");
    METASOUND_PARAM(OutA, "Out Audio {0}", "Output audio {0}");
    METASOUND_PARAM(InK, "In Control {0}", "Input control {0}");
    METASOUND_PARAM(OutK, "Out Control {0}", "Output control {0}");

    class FCsoundOperator : public TExecutableOperator<FCsoundOperator>
    {
    public:
        // Constructor
        FCsoundOperator(const FOperatorSettings& InSettings,
            const FStringReadRef& FilePath,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const size_t& NumOutAudioChannels, // WIP Should I use uint8 or size_t?
            const TArray<FFloatReadRef>& InControlRefs,
            const size_t& NumOutControlChannels, // WIP Should I use uint8 or size_t?
            const FStringReadRef& EventString,
            const FTriggerReadRef& EventTrigger
            )
            : m_FilePath(FilePath)
            , m_EventString(EventString)
            , m_EventTrigger(EventTrigger)
            , m_InAudioRefs(InAudioRefs)
            , m_InBuffers()
            , m_OutAudioRefs()
            , m_OutBuffers()
            , m_InControlRefs(InControlRefs)
            , m_InControlNames()
            , m_OutControlRefs()
            , m_OutControlNames()
            , m_OpSettings(InSettings)
            , m_CsoundInstance()
            , m_SpIndex(0)
            , m_Spin()
            , m_Spout()
            , m_Finished(false)
            // More variables still missing WIP
        {
            const char* csdFilePath = StringCast<ANSICHAR>(**m_FilePath.Get()).Get();
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
            
            /*
            GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,
                FString::Printf(TEXT("sr: %f,csound sr: %f")
                    , m_OpSettings.GetSampleRate()
                    , m_CsoundInstance.GetSr()));*/

            m_InBuffers.Empty(m_InAudioRefs.Num());
            for (size_t i = 0; i < m_InAudioRefs.Num(); i++)
            {
                m_InBuffers.Add(m_InAudioRefs[i]->GetData());
            }

            m_OutAudioRefs.Empty(NumOutAudioChannels);
            m_OutBuffers.Empty(NumOutAudioChannels);
            for (size_t i = 0; i < NumOutAudioChannels; i++)
            {
                m_OutAudioRefs.Add(FAudioBufferWriteRef::CreateNew(m_OpSettings));
                m_OutBuffers.Add(m_OutAudioRefs[i]->GetData());
            }

            m_InControlNames.Empty(m_InControlRefs.Num());
            //m_InControlNames2.Empty(m_InControlRefs.Num());
            for (size_t i = 0; i < m_InControlRefs.Num(); i++)
            {
                m_InControlNames.Add("InK_" + FString::FromInt(i)); // WIP Change name
                //m_InControlNames2[i] = new char[20];
                //strcpy(m_InControlNames2[i] , StringCast<ANSICHAR>(*m_InControlNames[i]).Get());
            }

            m_OutControlRefs.Empty(NumOutControlChannels);
            m_OutControlNames.Empty(NumOutControlChannels);
            for (size_t i = 0; i < NumOutControlChannels; i++)
            {
                m_OutControlRefs.Add(FFloatWriteRef::CreateNew());
                m_OutControlNames.Add("OutK_" + FString::FromInt(i));
            }

            m_CsoundNchnlsIn = m_CsoundInstance.GetNchnlsInput();
            m_CsoundNchnlsOut = m_CsoundInstance.GetNchnls();
            m_MinAudioIn = m_InBuffers.Num() <= m_CsoundNchnlsIn ? m_InBuffers.Num() : m_CsoundNchnlsIn;
            m_MinAudioOut = m_OutBuffers.Num() <= m_CsoundNchnlsOut ? m_OutBuffers.Num() : m_CsoundNchnlsOut;
            m_CsoundKsmps = (uint32)m_CsoundInstance.GetKsmps();

            // No way to subscribe in advance?
            //const FTrigger *trig = m_EventTrigger.Get();
            //trig->
            
        };

        // Primary node functionality
        void Execute()
        {
            if (m_Finished) return;

            if (m_EventTrigger->IsTriggeredInBlock())
            {
                // WIP Does Csound work in sample accurate score line events? Use a string queue, handled by another node
                // Or the best way would be to use callbacks, does m_EventTrigger allow for callbacks??
                // Not sample accurate
                const char* eventCString = StringCast<ANSICHAR>(**m_EventString.Get()).Get();
                m_CsoundInstance.InputMessage(eventCString);
            }

            for (size_t f = 0; f < m_OpSettings.GetNumFramesPerBlock(); f++)
            {
                for (size_t i = 0; i < m_MinAudioIn; i++)
                {
                    // WIP Use version of Csound that uses floats instead of doubles?
                    m_Spin[m_SpIndex * m_CsoundNchnlsIn + i] = (double)(m_InBuffers[i][f]);
                }
                
                for (size_t i = 0; i < m_MinAudioOut; i++)
                {
                    m_OutBuffers[i][f] = (float)m_Spout[m_SpIndex * m_CsoundNchnlsOut + i];
                }

                if (++m_SpIndex >= m_CsoundKsmps)
                {
                    for (size_t i = 0; i < m_InControlNames.Num(); i++)
                    {
                        // Use FString::Format instead of keeping the strings in an array?
                        m_CsoundInstance.SetControlChannel(StringCast<ANSICHAR>(*m_InControlNames[i]).Get(), (double)*(m_InControlRefs[i]));
                    }

                    m_Finished = (bool)m_CsoundInstance.PerformKsmps();
                    m_SpIndex = 0;

                    if (m_Finished)
                    {
                        ClearChannels();
                        return;
                    }
                    
                    for (size_t i = 0; i < m_OutControlRefs.Num(); i++)
                    {
                        *(m_OutControlRefs[i]) = (float)m_CsoundInstance.GetControlChannel(StringCast<ANSICHAR>(*m_OutControlNames[i]).Get());
                    }

                    // csound.destroy and csound.compile to recompile to make it like a loop
                }
            }
        }

        void ClearChannels()
        {
            for (size_t i = 0; i < m_OutAudioRefs.Num(); i++)
                m_OutAudioRefs[i]->Zero();

            for (size_t i = 0; i < m_OutControlRefs.Num(); i++)
                *m_OutControlRefs[i] = 0.;
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
                    Info.Author = PluginAuthor; // WIP Add my name here
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
            FInputVertexInterface inVer;
            inVer.Add(TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(CsoundFilePath)));

            for (uint32 i = 0; i < _numAudioChannelsIn; i++)
                inVer.Add(TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(InA, i)));
            for (uint32 i = 0; i < _numControlChannelsIn; i++)
                inVer.Add(TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(InK, i)));

            inVer.Add(TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(EventString)));
            inVer.Add(TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(EventTrigger)));
            
            FOutputVertexInterface outVer;
            for (uint32 i = 0; i < _numAudioChannelsOut; i++)
                outVer.Add(TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(OutA, i)));
            for (uint32 i = 0; i < _numControlChannelsOut; i++)
                outVer.Add(TOutputDataVertex<float>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(OutK, i)));

            static const FVertexInterface Interface(inVer, outVer);
            return Interface;
        }

        // Allows MetaSound graph to interact with your node's inputs
        virtual FDataReferenceCollection GetInputs() const override
        {
            FDataReferenceCollection InputDataReferences;

            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(CsoundFilePath), m_FilePath);

            for (uint32 i = 0; i < _numAudioChannelsIn; i++)
                InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(InA, i), m_InAudioRefs[i]);
            for (uint32 i = 0; i < _numControlChannelsIn; i++)
                InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(InK, i), m_InControlRefs[i]);

            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(EventString), m_EventString);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(EventTrigger), m_EventTrigger);

            return InputDataReferences;
        }

        // Allows MetaSound graph to interact with your node's outputs
        virtual FDataReferenceCollection GetOutputs() const override
        {
            FDataReferenceCollection OutputDataReferences;

            for (uint32 i = 0; i < _numAudioChannelsOut; i++)
                OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(OutA, i), m_OutAudioRefs[i]);
            for (uint32 i = 0; i < _numControlChannelsOut; i++)
                OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(OutK, i), m_OutControlRefs[i]);

            return OutputDataReferences;
        }

        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors)
        {
            const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
            const FInputVertexInterface& InputInterface = DeclareVertexInterface().GetInputInterface();

            TDataReadReference<FString> CsoundFP = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>
                (InputInterface, METASOUND_GET_PARAM_NAME(CsoundFilePath), InParams.OperatorSettings);
            TDataReadReference<FString> EvString = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>
                (InputInterface, METASOUND_GET_PARAM_NAME(EventString), InParams.OperatorSettings);
            TDataReadReference<FTrigger> EvTrigger = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FTrigger>
                (InputInterface, METASOUND_GET_PARAM_NAME(EventTrigger), InParams.OperatorSettings);

            TArray<TDataReadReference<FAudioBuffer>> arrayAudio;
            arrayAudio.Empty(_numAudioChannelsIn);
            for (uint32 i = 0; i < _numAudioChannelsIn; i++)
            { 
                arrayAudio.Add(TDataReadReference<FAudioBuffer>
                (
                    InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FAudioBuffer>
                    (
                        InputInterface,
                        METASOUND_GET_PARAM_NAME_WITH_INDEX(InA, i),
                        InParams.OperatorSettings
                    )
                ));
            }

            TArray<TDataReadReference<float>> arrayControl;
            arrayControl.Empty(_numControlChannelsIn);
            for (uint32 i = 0; i < _numControlChannelsIn; i++)
            {
                arrayControl.Add(TDataReadReference<float>
                (
                    InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>
                    (
                        InputInterface,
                        METASOUND_GET_PARAM_NAME_WITH_INDEX(InK, i),
                        InParams.OperatorSettings
                    )
                ));
            }

            return MakeUnique<FCsoundOperator>(
                InParams.OperatorSettings,
                CsoundFP,
                arrayAudio,
                _numAudioChannelsOut,
                arrayControl,
                _numControlChannelsOut,
                EvString,
                EvTrigger
            );
        }

    private:
        FStringReadRef m_FilePath;
        FStringReadRef m_EventString;
        FTriggerReadRef m_EventTrigger;

        TArray<FAudioBufferReadRef> m_InAudioRefs;
        TArray<const float*> m_InBuffers;
        TArray<FAudioBufferWriteRef> m_OutAudioRefs;
        TArray<float*> m_OutBuffers;

        TArray<FFloatReadRef> m_InControlRefs;
        TArray<FString> m_InControlNames;
        TArray<FFloatWriteRef> m_OutControlRefs;
        TArray<FString> m_OutControlNames;

        FOperatorSettings m_OpSettings;
        Csound m_CsoundInstance;
        uint32 m_SpIndex;
        bool m_Finished;

        double *m_Spin, *m_Spout;
        size_t m_CsoundNchnlsIn, m_CsoundNchnlsOut, m_MinAudioIn, m_MinAudioOut;
        uint32 m_CsoundKsmps;

        static const uint32 _numAudioChannelsIn = 2; // WIP Change name and formatting?
        static const uint32 _numAudioChannelsOut = 2;
        static const uint32 _numControlChannelsIn = 2;
        static const uint32 _numControlChannelsOut = 1;
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
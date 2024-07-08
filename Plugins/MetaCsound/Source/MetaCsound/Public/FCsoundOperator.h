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

    METASOUND_PARAM(FilePath, "File", "Path of the .csd file to be executed by Csound");
    METASOUND_PARAM(EvStr, "Event String", "The string that contains a Csound event");
    METASOUND_PARAM(EvTrig, "Event Trigger", "Triggers the Csound event descrived by EventString");
    METASOUND_PARAM(FinTrig, "On Finished", "Triggers when the Csound score has finished");

    METASOUND_PARAM(InA, "In Audio {0}", "Input audio {0}");
    METASOUND_PARAM(OutA, "Out Audio {0}", "Output audio {0}");
    METASOUND_PARAM(InK, "In Control {0}", "Input control {0}");
    METASOUND_PARAM(OutK, "Out Control {0}", "Output control {0}");

    class FCsoundOperator : public TExecutableOperator<FCsoundOperator>
    {
    public:
        // Constructor
        FCsoundOperator(const FOperatorSettings& InSettings,
            const FStringReadRef& InFilePath,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const size_t& InNumOutAudioChannels, // WIP Should I use uint8 or size_t?
            const TArray<FFloatReadRef>& InControlRefs,
            const size_t& InNumOutControlChannels, // WIP Should I use uint8 or size_t?
            const FStringReadRef& InEventString,
            const FTriggerReadRef& InEventTrigger
            )
            : FilePath(InFilePath)
            , EventString(InEventString)
            , EventTrigger(InEventTrigger)
            , FinishedTrigger(FTriggerWriteRef::CreateNew(InSettings))
            , AudioInRefs(InAudioRefs)
            , BuffersIn()
            , AudioOutRefs()
            , BuffersOut()
            , ControlInRefs(InControlRefs)
            , ControlInNames()
            , ControlOutRefs()
            , ControlOutNames()
            , OpSettings(InSettings)
            , CsoundInstance()
            , SpIndex(0)
            , Spin()
            , Spout()
            , bFinished(false)
            // More variables still missing WIP
        {
            const char* CsdFilePath = StringCast<ANSICHAR>(**FilePath.Get()).Get();
            FString SrOptionFString = "--sample-rate=" + FString::FromInt((int)OpSettings.GetSampleRate());
            const char* SrOption = StringCast<ANSICHAR>(*SrOptionFString).Get();

            // WIP make not explode it when compilation error
            CsoundInstance.Compile(CsdFilePath, SrOption, "-n");

            Spin = CsoundInstance.GetSpin();
            Spout = CsoundInstance.GetSpout();
            
            // WIP Make schedule "f0 z" so it runs indefinetely independently of the score, it's not working right now
            // Csound Unity does not have this option either, the user has to add it manually to the csd file
            // --daemon to do that
            CsoundInstance.InputMessage("f0 z");

            // WIP Create a stop event?
            // WIP when Csound stops, an annoying sound gets outputed, stop that
            
            /*
            GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,
                FString::Printf(TEXT("sr: %f,csound sr: %f")
                    , m_OpSettings.GetSampleRate()
                    , m_CsoundInstance.GetSr()));*/

            BuffersIn.Empty(InAudioRefs.Num());
            for (size_t i = 0; i < InAudioRefs.Num(); i++)
            {
                BuffersIn.Add(InAudioRefs[i]->GetData());
            }

            AudioOutRefs.Empty(InNumOutAudioChannels);
            BuffersOut.Empty(InNumOutAudioChannels);
            for (size_t i = 0; i < InNumOutAudioChannels; i++)
            {
                AudioOutRefs.Add(FAudioBufferWriteRef::CreateNew(OpSettings));
                BuffersOut.Add(AudioOutRefs[i]->GetData());
            }

            ControlInNames.Empty(ControlInRefs.Num());
            for (size_t i = 0; i < ControlInRefs.Num(); i++)
            {
                ControlInNames.Add("InK_" + FString::FromInt(i)); // WIP Change name
            }

            ControlOutRefs.Empty(InNumOutControlChannels);
            ControlOutNames.Empty(InNumOutControlChannels);
            for (size_t i = 0; i < InNumOutControlChannels; i++)
            {
                ControlOutRefs.Add(FFloatWriteRef::CreateNew());
                ControlOutNames.Add("OutK_" + FString::FromInt(i));
            }

            CsoundNchnlsIn = CsoundInstance.GetNchnlsInput();
            CsoundNchnlsOut = CsoundInstance.GetNchnls();
            MinAudioIn = BuffersIn.Num() <= CsoundNchnlsIn ? BuffersIn.Num() : CsoundNchnlsIn;
            MinAudioOut = BuffersOut.Num() <= CsoundNchnlsOut ? BuffersOut.Num() : CsoundNchnlsOut;
            CsoundKsmps = (uint32)CsoundInstance.GetKsmps();

            // No way to subscribe in advance?
            //const FTrigger *trig = m_EventTrigger.Get();
            //trig->
            
        };

        // Primary node functionality
        void Execute()
        {
            if (bFinished)
            {
                FinishedTrigger->AdvanceBlock();
                return;
            }
                

            if (EventTrigger->IsTriggeredInBlock())
            {
                // WIP Does Csound work in sample accurate score line events? Use a string queue, handled by another node
                // Or the best way would be to use callbacks, does m_EventTrigger allow for callbacks??
                // Not sample accurate
                const char* EventCString = StringCast<ANSICHAR>(**EventString.Get()).Get();
                CsoundInstance.InputMessage(EventCString);
            }

            for (size_t f = 0; f < OpSettings.GetNumFramesPerBlock(); f++)
            {
                for (size_t i = 0; i < MinAudioIn; i++)
                {
                    // WIP Use version of Csound that uses floats instead of doubles?
                    Spin[SpIndex * CsoundNchnlsIn + i] = (double)(BuffersIn[i][f]);
                }
                
                for (size_t i = 0; i < MinAudioOut; i++)
                {
                    BuffersOut[i][f] = (float)Spout[SpIndex * CsoundNchnlsOut + i];
                }

                if (++SpIndex >= CsoundKsmps)
                {
                    for (size_t i = 0; i < ControlInNames.Num(); i++)
                    {
                        // Use FString::Format instead of keeping the strings in an array?
                        CsoundInstance.SetControlChannel(StringCast<ANSICHAR>(*ControlInNames[i]).Get(), (double)*(ControlInRefs[i]));
                    }

                    bFinished = (bool)CsoundInstance.PerformKsmps();
                    SpIndex = 0;

                    if (bFinished)
                    {
                        FinishedTrigger->TriggerFrame(f);
                        ClearChannels();
                        return;
                    }
                    
                    for (size_t i = 0; i < ControlOutRefs.Num(); i++)
                    {
                        *(ControlOutRefs[i]) = (float)CsoundInstance.GetControlChannel(StringCast<ANSICHAR>(*ControlOutNames[i]).Get());
                    }

                    // csound.destroy and csound.compile to recompile to make it like a loop
                }
            }
        }

        void ClearChannels()
        {
            for (size_t i = 0; i < AudioOutRefs.Num(); i++)
            {
                AudioOutRefs[i]->Zero();
            }
                
            for (size_t i = 0; i < ControlOutRefs.Num(); i++)
            {
                *ControlOutRefs[i] = 0.;
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
                    Info.Author = PluginAuthor; // WIP Add my name here
                    Info.PromptIfMissing = PluginNodeMissingPrompt;
                    Info.DefaultInterface = DeclareVertexInterface();
                    Info.CategoryHierarchy = { LOCTEXT("Metasound_CsoundNodeCategory", "Utils") };

                    return Info;
                };

            // WIP put this as a static member of the class?
            static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
            return Metadata;
        }

        static const FVertexInterface& DeclareVertexInterface()
        {
            FInputVertexInterface InputVertex;
            InputVertex.Add(TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(FilePath)));

            for (uint32 i = 0; i < NumAudioChannelsIn; i++)
            {
                InputVertex.Add(TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(InA, i)));
            }
                
            for (uint32 i = 0; i < NumControlChannelsIn; i++)
            {
                InputVertex.Add(TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(InK, i)));
            }
                

            InputVertex.Add(TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(EvStr)));
            InputVertex.Add(TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(EvTrig)));
            
            FOutputVertexInterface OutputVertex;
            OutputVertex.Add(TOutputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(FinTrig)));
            for (uint32 i = 0; i < NumAudioChannelsOut; i++)
            {
                OutputVertex.Add(TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(OutA, i)));
            }
                
            for (uint32 i = 0; i < NumControlChannelsOut; i++)     
            {
                OutputVertex.Add(TOutputDataVertex<float>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(OutK, i)));
            }

            static const FVertexInterface VertexInterface(InputVertex, OutputVertex);
            return VertexInterface;
        }

        // Allows MetaSound graph to interact with your node's inputs
        virtual FDataReferenceCollection GetInputs() const override final
        {
            FDataReferenceCollection InputDataReferences;

            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(FilePath), FilePath);

            for (size_t i = 0; i < AudioInRefs.Num(); i++)
            {
                InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(InA, i), AudioInRefs[i]);
            }
                
            for (size_t i = 0; i < ControlInRefs.Num(); i++)
            {
                InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(InK, i), ControlInRefs[i]);
            }

            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(EvStr), EventString);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(EvTrig), EventTrigger);

            return InputDataReferences;
        }

        // Allows MetaSound graph to interact with your node's outputs
        virtual FDataReferenceCollection GetOutputs() const override final
        {
            FDataReferenceCollection OutputDataReferences;

            OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(FinTrig), FinishedTrigger);

            for (size_t i = 0; i < AudioOutRefs.Num(); i++)
            {
                OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(OutA, i), AudioOutRefs[i]);
            }
                
            for (size_t i = 0; i < ControlOutRefs.Num(); i++)
            {
                OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(OutK, i), ControlOutRefs[i]);
            }

            return OutputDataReferences;
        }

        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors)
        {
            const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
            const FInputVertexInterface& InputInterface = DeclareVertexInterface().GetInputInterface();

            TDataReadReference<FString> CsoundFP = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>
                (InputInterface, METASOUND_GET_PARAM_NAME(FilePath), InParams.OperatorSettings);
            TDataReadReference<FString> EvString = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>
                (InputInterface, METASOUND_GET_PARAM_NAME(EvStr), InParams.OperatorSettings);
            TDataReadReference<FTrigger> EvTrigger = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FTrigger>
                (InputInterface, METASOUND_GET_PARAM_NAME(EvTrig), InParams.OperatorSettings);

            TArray<TDataReadReference<FAudioBuffer>> AudioInArray;
            AudioInArray.Empty(NumAudioChannelsIn);
            for (uint32 i = 0; i < NumAudioChannelsIn; i++)
            { 
                AudioInArray.Add(TDataReadReference<FAudioBuffer>
                (
                    InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FAudioBuffer>
                    (
                        InputInterface,
                        METASOUND_GET_PARAM_NAME_WITH_INDEX(InA, i),
                        InParams.OperatorSettings
                    )
                ));
            }

            TArray<TDataReadReference<float>> ControlInArray;
            ControlInArray.Empty(NumControlChannelsIn);
            for (uint32 i = 0; i < NumControlChannelsIn; i++)
            {
                ControlInArray.Add(TDataReadReference<float>
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
                AudioInArray,
                NumAudioChannelsOut,
                ControlInArray,
                NumControlChannelsOut,
                EvString,
                EvTrigger
            );
        }

    private:
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
        uint32 SpIndex;
        bool bFinished;

        double* Spin, * Spout;
        size_t CsoundNchnlsIn, CsoundNchnlsOut, MinAudioIn, MinAudioOut;
        uint32 CsoundKsmps;

        static const uint32 NumAudioChannelsIn = 2;
        static const uint32 NumAudioChannelsOut = 2;
        static const uint32 NumControlChannelsIn = 2;
        static const uint32 NumControlChannelsOut = 1;
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

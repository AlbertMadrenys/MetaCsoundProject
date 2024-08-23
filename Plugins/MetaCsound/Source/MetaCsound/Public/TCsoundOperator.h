/////////////////////////////////////////////////////////////////////
// TCsoundOperator template: base class Csound node operator
// 
// Copyright (C) 2024 Albert Madrenys
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.
//
/////////////////////////////////////////////////////////////////////
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
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros
#include "MetasoundFacade.h"				 // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "Containers/Array.h"

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetaCsound_TCsoundOperator"

namespace MetaCsound
{

    namespace NodeParams
    {
        METASOUND_PARAM(PlayTrig, "Play", "Starts playing Csound");
        METASOUND_PARAM(StopTrig, "Stop", "Stops the Csound performace");
        METASOUND_PARAM(CsoundFile, "File", "Name of the .csd file to be executed by Csound");
        METASOUND_PARAM(FinTrig, "On Finished", "Triggers when the Csound score has finished");

        METASOUND_PARAM(InA, "In Audio {0}", "Input audio {0}");
        METASOUND_PARAM(OutA, "Out Audio {0}", "Output audio {0}");
        METASOUND_PARAM(InK, "In Control {0}", "Input control {0}");
        METASOUND_PARAM(OutK, "Out Control {0}", "Output control {0}");
    }

    using namespace Metasound;

    template<typename DerivedOperator>
    class TCsoundOperator : public TExecutableOperator<DerivedOperator>
    {

    protected:
        // Protected constructor
        TCsoundOperator(
            const FOperatorSettings& InSettings,
            const FTriggerReadRef& InPlayTrigger,
            const FTriggerReadRef& InStopTrigger,
            const FStringReadRef& InCsoundFile,
            const TArray<FAudioBufferReadRef>& InAudioRefs,
            const int32& InNumOutAudioChannels,
            const TArray<FFloatReadRef>& InControlRefs,
            const int32& InNumOutControlChannels)
            : PlayTrigger(InPlayTrigger)
            , StopTrigger(InStopTrigger)
            , CsoundFile(InCsoundFile)
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
            , Spin(nullptr)
            , Spout(nullptr)
            , FirstClearedFrame(InSettings.GetNumFramesPerBlock())
            , OpState(EOpState::Stopped)
        {

            /*
            GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,
                FString::Printf(TEXT("sr: %f,csound sr: %f")
                    , m_OpSettings.GetSampleRate()
                    , m_CsoundInstance.GetSr()));*/

            BuffersIn.Empty(InAudioRefs.Num());
            for (int32 i = 0; i < InAudioRefs.Num(); i++)
            {
                BuffersIn.Add(InAudioRefs[i]->GetData());
            }

            AudioOutRefs.Empty(InNumOutAudioChannels);
            BuffersOut.Empty(InNumOutAudioChannels);
            for (int32 i = 0; i < InNumOutAudioChannels; i++)
            {
                AudioOutRefs.Add(FAudioBufferWriteRef::CreateNew(OpSettings));
                BuffersOut.Add(AudioOutRefs[i]->GetData());
            }

            ControlInNames.Empty(ControlInRefs.Num());
            for (int32 i = 0; i < ControlInRefs.Num(); i++)
            {
                ControlInNames.Add("In Control " + FString::FromInt(i));
            }

            ControlOutRefs.Empty(InNumOutControlChannels);
            ControlOutNames.Empty(InNumOutControlChannels);
            for (int32 i = 0; i < InNumOutControlChannels; i++)
            {
                ControlOutRefs.Add(FFloatWriteRef::CreateNew());
                ControlOutNames.Add("Out Control " + FString::FromInt(i));
            }
        }

    public:

        // Primary node functionality
        void Execute()
        {
            if (OpState == EOpState::Error)
            {
                return;
            }
            else if (OpState == EOpState::Stopped)
            {
                ClearChannels(0);
            }

            FinishedTrigger->AdvanceBlock();

            for (int32 f = 0; f < OpSettings.GetNumFramesPerBlock(); f++)
            {
                for (int32 i = 0; i < PlayTrigger->Num(); i++)
                {
                    if ((*PlayTrigger)[i] == f)
                    {
                        Play(f);
                        if (OpState == EOpState::Error)
                        {
                            return;
                        }
                        break;
                    }
                }

                for (int32 i = 0; i < StopTrigger->Num() && OpState != EOpState::Stopped; i++)
                {
                    if ((*StopTrigger)[i] == f)
                    {
                        Stop(f);
                        break;
                    }
                }

                if (OpState != EOpState::Playing)
                {
                    continue;
                }

                for (int32 i = 0; i < MinAudioIn; i++)
                {
                    Spin[SpIndex * CsoundNchnlsIn + i] = (double)(BuffersIn[i][f]);
                }

                for (int32 i = 0; i < MinAudioOut; i++)
                {
                    BuffersOut[i][f] = (float)Spout[SpIndex * CsoundNchnlsOut + i];
                }

                if (++SpIndex >= CsoundKsmps)
                {
                    for (int32 i = 0; i < ControlInRefs.Num(); i++)
                    {
                        // TODO: Use FString::Format instead of keeping the strings in an array?
                        CsoundInstance.SetControlChannel(StringCast<ANSICHAR>(*ControlInNames[i]).Get(), (double)*(ControlInRefs[i]));
                    }

                    CsoundPerformKsmps(f);

                    for (int32 i = 0; i < ControlOutRefs.Num() && OpState == EOpState::Playing; i++)
                    {
                        *(ControlOutRefs[i]) = (float)CsoundInstance.GetControlChannel(StringCast<ANSICHAR>(*ControlOutNames[i]).Get());
                    }
                }
            }
        }

        static const FNodeClassMetadata& GetNodeInfo()
        {
            auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
                {
                    FNodeClassMetadata Info;
                    Info.ClassName = DerivedOperator::GetClassName();
                    Info.MajorVersion = 1;
                    Info.MinorVersion = 0;
                    Info.DisplayName = DerivedOperator::GetDisplayName();
                    Info.Description = DerivedOperator::GetDescription();
                    Info.Author = TEXT("Albert Madrenys Planas");
                    Info.PromptIfMissing = Metasound::PluginNodeMissingPrompt;
                    Info.DefaultInterface = DeclareVertexInterface();
                    Info.CategoryHierarchy = { LOCTEXT("MetaCsound_NodeCategory", "Csound") };

                    return Info;
                };

            // WIP put this as a static member of the class?
            static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
            return Metadata;
        }

        static const FVertexInterface& DeclareVertexInterface()
        {
            using namespace NodeParams;

            FInputVertexInterface InputVertex;
            InputVertex.Add(TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(PlayTrig)));
            InputVertex.Add(TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(StopTrig)));
            InputVertex.Add(TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(CsoundFile)));
            for (int32 i = 0; i < DerivedOperator::NumAudioChannelsIn; i++)
            {
                InputVertex.Add(TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(InA, i)));
            }

            for (int32 i = 0; i < DerivedOperator::NumControlChannelsIn; i++)
            {
                InputVertex.Add(TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(InK, i)));
            }

            FOutputVertexInterface OutputVertex;
            OutputVertex.Add(TOutputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(FinTrig)));
            for (int32 i = 0; i < DerivedOperator::NumAudioChannelsOut; i++)
            {
                OutputVertex.Add(TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(OutA, i)));
            }

            for (int32 i = 0; i < DerivedOperator::NumControlChannelsOut; i++)
            {
                OutputVertex.Add(TOutputDataVertex<float>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(OutK, i)));
            }

            static const FVertexInterface VertexInterface(InputVertex, OutputVertex);
            return VertexInterface;
        }

        // Allows MetaSound graph to interact with your node's inputs
        virtual void BindInputs(Metasound::FInputVertexInterfaceData& InOutVertexData) override final
        {
            // Seems like BindInputs and BindOutputs should be declared const,
            // but the base virtual method is not currently.

            using namespace NodeParams;

            InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(PlayTrig), PlayTrigger);
            InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(StopTrig), StopTrigger);
            InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(CsoundFile), CsoundFile);

            for (int32 i = 0; i < AudioInRefs.Num(); i++)
            {
                InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME_WITH_INDEX(InA, i), AudioInRefs[i]);
            }

            for (int32 i = 0; i < ControlInRefs.Num(); i++)
            {
                InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME_WITH_INDEX(InK, i), ControlInRefs[i]);
            }
        }

        // Allows MetaSound graph to interact with your node's outputs
        virtual void BindOutputs(Metasound::FOutputVertexInterfaceData& InOutVertexData) override final
        {
            using namespace NodeParams;

            InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(FinTrig), FinishedTrigger);

            for (int32 i = 0; i < AudioOutRefs.Num(); i++)
            {
                InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME_WITH_INDEX(OutA, i), AudioOutRefs[i]);
            }

            for (int32 i = 0; i < ControlOutRefs.Num(); i++)
            {
                InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME_WITH_INDEX(OutK, i), ControlOutRefs[i]);
            }
        }

        // Static factory method
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors)
        {
            using namespace NodeParams;

            const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
            const FInputVertexInterface& InputInterface = DeclareVertexInterface().GetInputInterface();

            TDataReadReference<FTrigger> PlayTrigger = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FTrigger>
                (InputInterface, METASOUND_GET_PARAM_NAME(PlayTrig), InParams.OperatorSettings);
            TDataReadReference<FTrigger> StopTrigger = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FTrigger>
                (InputInterface, METASOUND_GET_PARAM_NAME(StopTrig), InParams.OperatorSettings);
            TDataReadReference<FString> CsoundFileString = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>
                (InputInterface, METASOUND_GET_PARAM_NAME(CsoundFile), InParams.OperatorSettings);

            TArray<TDataReadReference<FAudioBuffer>> AudioInArray;
            AudioInArray.Empty(DerivedOperator::NumAudioChannelsIn);
            for (int32 i = 0; i < DerivedOperator::NumAudioChannelsIn; i++)
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
            ControlInArray.Empty(DerivedOperator::NumControlChannelsIn);
            for (int32 i = 0; i < DerivedOperator::NumControlChannelsIn; i++)
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

            return MakeUnique<DerivedOperator>(
                InParams.OperatorSettings,
                PlayTrigger, StopTrigger,
                CsoundFileString,
                AudioInArray, DerivedOperator::NumAudioChannelsOut,
                ControlInArray, DerivedOperator::NumControlChannelsOut
            );
        }

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

        void Play(int32 CurrentFrame)
        {
            // TODO: Use CsoundInstance.RewindScore(), but its not important, compile is very quick
            CsoundInstance.Reset();

            const FString FullPath = FPaths::ProjectContentDir() + TEXT("CsoundFiles/") + *CsoundFile.Get() + TEXT(".csd");
            const char* FullPathANSI = StringCast<ANSICHAR>(*FullPath).Get();

            const FString SrOption = "--sample-rate=" + FString::FromInt((int)OpSettings.GetSampleRate());
            const char* SrOptionANSI = StringCast<ANSICHAR>(*SrOption).Get();

            const int32 ErrorCode = CsoundInstance.Compile(FullPathANSI, SrOptionANSI, "-n");
            //CsoundInstance.SetOption(SrOptionANSI);
            //CsoundInstance.SetOption("-n");
            //const int32 ErrorCode = CsoundInstance.CompileCsd(FullPathANSI);

            if (ErrorCode != 0)
            {
                // TODO: Use CsoundInstance.CreateMessageBuffer() to capture the error
                UE_LOG(LogMetaSound, Error, TEXT("Not able to compile Csound file: %s"), *FullPath);
                OpState = EOpState::Error;
                return;
            }
            else
            {
                OpState = EOpState::Playing;
            }

            Spin = CsoundInstance.GetSpin();
            Spout = CsoundInstance.GetSpout();

            CsoundNchnlsIn = CsoundInstance.GetNchnlsInput();
            CsoundNchnlsOut = CsoundInstance.GetNchnls();
            MinAudioIn = BuffersIn.Num() <= CsoundNchnlsIn ? BuffersIn.Num() : CsoundNchnlsIn;
            MinAudioOut = BuffersOut.Num() <= CsoundNchnlsOut ? BuffersOut.Num() : CsoundNchnlsOut;
            CsoundKsmps = (uint32)CsoundInstance.GetKsmps();
            FirstClearedFrame = OpSettings.GetNumFramesPerBlock();

            CsoundPerformKsmps(CurrentFrame);
        }


        void Stop(int32 StopFrame = 0)
        {
            OpState = EOpState::Stopped;

            ClearChannels(StopFrame);
            FinishedTrigger->TriggerFrame(StopFrame);
        }

        void ClearChannels(int32 StartClearingFrame = 0)
        {
            if (FirstClearedFrame <= StartClearingFrame)
            {
                // Already cleared
                return;
            }

            for (int32 i = 0; i < AudioOutRefs.Num(); i++)
            {
                if (StartClearingFrame == 0)
                {
                    AudioOutRefs[i]->Zero();
                }
                else
                {
                    for (int32 f = StartClearingFrame; f < OpSettings.GetNumFramesPerBlock(); f++)
                    {
                        BuffersOut[i][f] = 0.; // WIP do this with iterators or an available method?
                    }
                }
            }

            FirstClearedFrame = StartClearingFrame;

            for (int32 i = 0; i < ControlOutRefs.Num(); i++)
            {
                *ControlOutRefs[i] = 0.;
            }
        }

        void CsoundPerformKsmps(int32 CurrentFrame)
        {
            SpIndex = 0;
            if (CsoundInstance.PerformKsmps() != 0)
            {
                Stop(CurrentFrame);
            }
        }
    };
    
}

#undef LOCTEXT_NAMESPACE

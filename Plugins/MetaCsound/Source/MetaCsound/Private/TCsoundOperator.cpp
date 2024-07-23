// Fill out your copyright notice in the Description page of Project Settings.

#include "TCsoundOperator.h"

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
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros
#include "Containers/Array.h"

// WIP trying to create my own pin type
#include "MetasoundDataReference.h"
#include "MetasoundDataTypeRegistrationMacro.h"
#include "MetasoundDataReferenceMacro.h"
#include "MetasoundVariable.h"

#include "MetaCsound.h"

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetaCsound_CsoundNode"

template<typename DerivedOperator>
Metasound::TCsoundOperator<DerivedOperator>::TCsoundOperator(
    const FOperatorSettings& InSettings,
    const FTriggerReadRef& InPlayTrigger,
    const FTriggerReadRef& InStopTrigger,
    const FStringReadRef& InFilePath,
    const TArray<FAudioBufferReadRef>& InAudioRefs,
    const int32& InNumOutAudioChannels,
    const TArray<FFloatReadRef>& InControlRefs,
    const int32& InNumOutControlChannels,
    const FStringReadRef& InEventString,
    const FTriggerReadRef& InEventTrigger,
    const FCharReadRef& InCharReadRef)
    : PlayTrigger(InPlayTrigger)
    , StopTrigger(InStopTrigger)
    , FilePath(InFilePath)
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
    , Spin(nullptr)
    , Spout(nullptr)
    , OpState(EOpState::Stopped)
    , CharReadRef(InCharReadRef)
{
    // WIP Trying to make my own pin type
    TDataReferenceTypeInfo<TCHAR>::TypeId;
    FCharTypeInfo::TypeId;
    //Metasound::FBoolTypeInfo::TypeName;
    //Metasound::FCharReadRef readRef = Metasound::FCharReadRef(Metasound::FCharReadRef::CreateNew());
    //Metasound::FCharReadRef readRef = Metasound::FCharReadRef::CreateNew();

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
        ControlInNames.Add("InK_" + FString::FromInt(i)); // WIP Change name
    }

    ControlOutRefs.Empty(InNumOutControlChannels);
    ControlOutNames.Empty(InNumOutControlChannels);
    for (int32 i = 0; i < InNumOutControlChannels; i++)
    {
        ControlOutRefs.Add(FFloatWriteRef::CreateNew());
        ControlOutNames.Add("OutK_" + FString::FromInt(i));
    }
}

template<typename DerivedOperator>
void Metasound::TCsoundOperator<DerivedOperator>::Execute()
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

        // WIP all of this code into their own method?
        for (int32 i = 0; i < EventTrigger->Num(); i++)
        {
            if ((*EventTrigger)[i] == f)
            {
                // WIP The string might not be the correct one if EventTrigger->Num() > 1
                const char* EventCString = StringCast<ANSICHAR>(**EventString.Get()).Get();
                CsoundInstance.InputMessage(EventCString);
            }
        }

        for (int32 i = 0; i < MinAudioIn; i++)
        {
            // WIP Use version of Csound that uses floats instead of doubles?
            Spin[SpIndex * CsoundNchnlsIn + i] = (double)(BuffersIn[i][f]);
        }

        for (int32 i = 0; i < MinAudioOut; i++)
        {
            BuffersOut[i][f] = (float)Spout[SpIndex * CsoundNchnlsOut + i];
        }

        if (++SpIndex >= CsoundKsmps)
        {
            for (int32 i = 0; i < ControlInNames.Num(); i++)
            {
                // Use FString::Format instead of keeping the strings in an array?
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

template<typename DerivedOperator>
void Metasound::TCsoundOperator<DerivedOperator>::Play(int32 CurrentFrame)
{
    CsoundInstance.Reset();

    const char* CsdFilePath = StringCast<ANSICHAR>(**FilePath.Get()).Get();
    FString SrOptionFString = "--sample-rate=" + FString::FromInt((int)OpSettings.GetSampleRate());
    const char* SrOption = StringCast<ANSICHAR>(*SrOptionFString).Get();

    // WIP Try to only compile one time, not on every Play call - use rewind score???
    int32 ErrorCode = CsoundInstance.Compile(CsdFilePath, SrOption, "-n");
    if (ErrorCode != 0)
    {
        // WIP Use CsoundInstance.CreateMessageBuffer() to get the error?
        UE_LOG(LogMetaSound, Error, TEXT("Not able to compile Csound file: %s"), **FilePath.Get());
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

    CsoundPerformKsmps(CurrentFrame);
}

template<typename DerivedOperator>
void Metasound::TCsoundOperator<DerivedOperator>::Stop(int32 StopFrame)
{
    OpState = EOpState::Stopped;

    ClearChannels(StopFrame);
    FinishedTrigger->TriggerFrame(StopFrame);
}

template<typename DerivedOperator>
void Metasound::TCsoundOperator<DerivedOperator>::ClearChannels(int32 StopFrame)
{
    // WIP add bool to know the previous StopFrame value? 
    for (int32 i = 0; i < AudioOutRefs.Num(); i++)
    {
        if (StopFrame == 0)
        {
            AudioOutRefs[i]->Zero();
        }
        else
        {
            for (int32 f = StopFrame; f < OpSettings.GetNumFramesPerBlock(); f++)
            {
                BuffersOut[i][f] = 0.; // WIP do this with iterators or an available method?
            }
        }
    }

    for (int32 i = 0; i < ControlOutRefs.Num(); i++)
    {
        *ControlOutRefs[i] = 0.;
    }
}

template<typename DerivedOperator>
void Metasound::TCsoundOperator<DerivedOperator>::CsoundPerformKsmps(int32 CurrentFrame)
{
    SpIndex = 0;
    if (CsoundInstance.PerformKsmps() != 0)
    {
        Stop(CurrentFrame);
    }
}

template<typename DerivedOperator>
const Metasound::FNodeClassMetadata& Metasound::TCsoundOperator<DerivedOperator>::GetNodeInfo()
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

template<typename DerivedOperator>
const Metasound::FVertexInterface& Metasound::TCsoundOperator<DerivedOperator>::DeclareVertexInterface()
{
    using namespace Metasound::CsoundNode;

    FInputVertexInterface InputVertex;
    InputVertex.Add(TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(PlayTrig)));
    InputVertex.Add(TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(StopTrig)));
    InputVertex.Add(TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(FilePath)));
    for (int32 i = 0; i < DerivedOperator::NumAudioChannelsIn; i++)
    {
        InputVertex.Add(TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(InA, i)));
    }

    for (int32 i = 0; i < DerivedOperator::NumControlChannelsIn; i++)
    {
        InputVertex.Add(TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_WITH_INDEX_AND_METADATA(InK, i)));
    }

    InputVertex.Add(TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(EvStr)));
    InputVertex.Add(TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(EvTrig)));

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

    InputVertex.Add(TInputDataVertex<TCHAR>(METASOUND_GET_PARAM_NAME_AND_METADATA(CharRead)));

    static const FVertexInterface VertexInterface(InputVertex, OutputVertex);
    return VertexInterface;
}

template<typename DerivedOperator>
Metasound::FDataReferenceCollection Metasound::TCsoundOperator<DerivedOperator>::GetInputs() const
{
    using namespace Metasound::CsoundNode;

    FDataReferenceCollection InputDataReferences;

    InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(PlayTrig), PlayTrigger);
    InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(StopTrig), StopTrigger);
    InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(FilePath), FilePath);

    for (int32 i = 0; i < AudioInRefs.Num(); i++)
    {
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(InA, i), AudioInRefs[i]);
    }

    for (int32 i = 0; i < ControlInRefs.Num(); i++)
    {
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(InK, i), ControlInRefs[i]);
    }

    InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(EvStr), EventString);
    InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(EvTrig), EventTrigger);

    InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(CharRead), CharReadRef);

    return InputDataReferences;
}

template<typename DerivedOperator>
Metasound::FDataReferenceCollection Metasound::TCsoundOperator<DerivedOperator>::GetOutputs() const
{
    using namespace Metasound::CsoundNode;

    FDataReferenceCollection OutputDataReferences;

    OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(FinTrig), FinishedTrigger);

    for (int32 i = 0; i < AudioOutRefs.Num(); i++)
    {
        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(OutA, i), AudioOutRefs[i]);
    }

    for (int32 i = 0; i < ControlOutRefs.Num(); i++)
    {
        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME_WITH_INDEX(OutK, i), ControlOutRefs[i]);
    }

    return OutputDataReferences;
}

template<typename DerivedOperator>
TUniquePtr<Metasound::IOperator> Metasound::TCsoundOperator<DerivedOperator>::CreateOperator(const FCreateOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors)
{
    using namespace Metasound::CsoundNode;

    const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
    const FInputVertexInterface& InputInterface = DeclareVertexInterface().GetInputInterface();

    TDataReadReference<FTrigger> PlayTrigger = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FTrigger>
        (InputInterface, METASOUND_GET_PARAM_NAME(PlayTrig), InParams.OperatorSettings);
    TDataReadReference<FTrigger> StopTrigger = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FTrigger>
        (InputInterface, METASOUND_GET_PARAM_NAME(StopTrig), InParams.OperatorSettings);
    TDataReadReference<FString> CsoundFP = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>
        (InputInterface, METASOUND_GET_PARAM_NAME(FilePath), InParams.OperatorSettings);
    TDataReadReference<FString> EvString = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>
        (InputInterface, METASOUND_GET_PARAM_NAME(EvStr), InParams.OperatorSettings);
    TDataReadReference<FTrigger> EvTrigger = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FTrigger>
        (InputInterface, METASOUND_GET_PARAM_NAME(EvTrig), InParams.OperatorSettings);

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

    TDataReadReference<TCHAR> CharReadRef = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<TCHAR>
        (InputInterface, METASOUND_GET_PARAM_NAME(CharRead), InParams.OperatorSettings);

    return MakeUnique<DerivedOperator>(
        InParams.OperatorSettings,
        PlayTrigger, StopTrigger,
        CsoundFP,
        AudioInArray, DerivedOperator::NumAudioChannelsOut,
        ControlInArray, DerivedOperator::NumControlChannelsOut,
        EvString, EvTrigger,
        CharReadRef
    );
}

#undef LOCTEXT_NAMESPACE

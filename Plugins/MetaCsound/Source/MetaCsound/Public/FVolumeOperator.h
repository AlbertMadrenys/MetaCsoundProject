#include "MetasoundExecutableOperator.h"     // TExecutableOperator class
#include "MetasoundPrimitives.h"             // ReadRef and WriteRef descriptions for bool, int32, float, and string
#include "MetasoundNodeRegistrationMacro.h"  // METASOUND_LOCTEXT and METASOUND_REGISTER_NODE macros
#include "MetasoundStandardNodesNames.h"     // StandardNodes namespace
#include "MetasoundFacade.h"				 // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetaCsound_VolumeNode"

namespace Metasound
{
    namespace VolumeNodeNames
    {
        METASOUND_PARAM(InputAudio, "In", "Input audio");
        METASOUND_PARAM(Amplitude, "Amp", "Amplitude to be applied to input audio");

        METASOUND_PARAM(OutputAudio, "Out", "Output audio");
    }

    class FVolumeOperator : public TExecutableOperator<FVolumeOperator>
    {
    public:
        // Constructor
        FVolumeOperator(const FOperatorSettings& InSettings,
            const FAudioBufferReadRef& InAudio,
            const FFloatReadRef& Amp)
            : m_Amp(Amp)
            , m_InAudio(InAudio)
            , m_OutAudio(FAudioBufferWriteRef::CreateNew(InSettings))
        { };

        static const FNodeClassMetadata& GetNodeInfo()
        {
            auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
                {
                    FVertexInterface NodeInterface = DeclareVertexInterface();

                    FNodeClassMetadata Info;

                    Info.ClassName = { TEXT("UE"), TEXT("Volume"), TEXT("Audio") };
                    Info.MajorVersion = 1;
                    Info.MinorVersion = 0;
                    Info.DisplayName = LOCTEXT("Metasound_VolumeNodeDisplayName", "Volume");
                    Info.Description = LOCTEXT("Metasound_VolumeNodeDesc", "Applies volume to the audio input.");
                    Info.Author = PluginAuthor;
                    Info.PromptIfMissing = PluginNodeMissingPrompt;
                    Info.DefaultInterface = DeclareVertexInterface();
                    Info.CategoryHierarchy = { LOCTEXT("Metasound_VolumeNodeCategory", "Utils") };

                    return Info;
                };

            static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
            return Metadata;
        }

        static const FVertexInterface& DeclareVertexInterface()
        {
            using namespace VolumeNodeNames;

            static const FVertexInterface Interface(
                FInputVertexInterface(
                    TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputAudio)),
                    TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(Amplitude))
                ),
                FOutputVertexInterface(
                    TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputAudio))
                )
            );

            return Interface;
        }

        // Allows MetaSound graph to interact with your node's inputs
        virtual FDataReferenceCollection GetInputs() const override
        {
            using namespace VolumeNodeNames;
            FDataReferenceCollection InputDataReferences;

            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(Amplitude), m_Amp);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InputAudio), m_InAudio);

            return InputDataReferences;
        }

        // Allows MetaSound graph to interact with your node's outputs
        virtual FDataReferenceCollection GetOutputs() const override
        {
            using namespace VolumeNodeNames;
            FDataReferenceCollection OutputDataReferences;

            OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutputAudio), m_OutAudio);

            return OutputDataReferences;
        }

        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors)
        {
            using namespace VolumeNodeNames;

            const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
            const FInputVertexInterface& InputInterface = DeclareVertexInterface().GetInputInterface();

            TDataReadReference<FAudioBuffer> InputAudio = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FAudioBuffer>(InputInterface,
                METASOUND_GET_PARAM_NAME(InputAudio), InParams.OperatorSettings);
            TDataReadReference<float> InputB = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface,
                METASOUND_GET_PARAM_NAME(Amplitude), InParams.OperatorSettings);

            return MakeUnique<FVolumeOperator>(InParams.OperatorSettings, InputAudio, InputB);
        }

        // Primary node functionality
        void Execute()
        {
            const float* in = m_InAudio->GetData();
            float* out = m_OutAudio->GetData();

            const int32 numSamples = m_InAudio->Num();

            for (int32 i = 0; i < numSamples; i++)
            {
                out[i] = *m_Amp * in[i];
            }
        }

    private:
        // Inputs
        FFloatReadRef m_Amp;
        FAudioBufferReadRef m_InAudio;

        // Outputs
        FAudioBufferWriteRef m_OutAudio;
    };

    class FVolumeNode : public FNodeFacade
    {
    public:
        FVolumeNode(const FNodeInitData& InitData) : FNodeFacade(InitData.InstanceName, InitData.InstanceID,
            TFacadeOperatorClass<FVolumeOperator>())
        {
        }
    };

    // Register node
    METASOUND_REGISTER_NODE(FVolumeNode);
}

#undef LOCTEXT_NAMESPACE
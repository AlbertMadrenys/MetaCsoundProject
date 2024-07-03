//#pragma once WIP

#include "MetasoundExecutableOperator.h"     // TExecutableOperator class
#include "MetasoundPrimitives.h"             // ReadRef and WriteRef descriptions for bool, int32, float, and string
#include "MetasoundNodeRegistrationMacro.h"  // METASOUND_LOCTEXT and METASOUND_REGISTER_NODE macros
#include "MetasoundStandardNodesNames.h"     // StandardNodes namespace
#include "MetasoundFacade.h"				 // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros
//#include "MetasoundNodeInterface.h"

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetaCsound_TutorialNode"

// WIP: Should I use namespace Metasound?
//namespace CsoundForUnreal
namespace Metasound
{
    // Vertex Names - define your node’s inputs and outputs here
    namespace TutorialNodeNames
    {
        METASOUND_PARAM(InputAValue, "A", "Input value A.");
        METASOUND_PARAM(InputBValue, "B", "Input value B.");

        METASOUND_PARAM(OutputValue, "Sum of A and B", "The sum of A and B.");
    }

    //using namespace Metasound;

    // WIP: Put the strange name that appears when you add a class?
    // Operator Class - defines the way your node is described, created and executed
    class FTutorialOperator : public TExecutableOperator<FTutorialOperator>
    {
    public:
        // Constructor
        FTutorialOperator(const FOperatorSettings& InSettings,
            const FFloatReadRef& InInputAValue,
            const FFloatReadRef& InInputBValue)
            : InputA(InInputAValue)
            , InputB(InInputBValue)
            , TutorialNodeOutput(FFloatWriteRef::CreateNew(*InputA + *InputB))
        { };

        static const FVertexInterface& DeclareVertexInterface()
        {
            using namespace TutorialNodeNames;

            static const FVertexInterface Interface(
                FInputVertexInterface(
                    TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputAValue)),
                    TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputBValue))
                ),
                FOutputVertexInterface(
                    TOutputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputValue))
                )
            );

            return Interface;
        }

        // Retrieves necessary metadata about your node
        static const FNodeClassMetadata& GetNodeInfo()
        {
            auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
                {
                    FVertexInterface NodeInterface = DeclareVertexInterface();

                    FNodeClassMetadata Metadata
                    {
                        FNodeClassName { StandardNodes::Namespace, "Tutorial Node",
                                StandardNodes::AudioVariant },
                        1, // Major Version
                        0, // Minor Version
                        METASOUND_LOCTEXT("TutorialNodeDisplayName", "Tutorial Node"),
                        METASOUND_LOCTEXT("TutorialNodeDesc", "A simple node to demonstrate how to create new MetaSound nodes in C++.Adds two floats together"),
                        PluginAuthor,
                        PluginNodeMissingPrompt,
                        NodeInterface,
                        { },
                        { },
                        FNodeDisplayStyle{}
                    };

                    return Metadata;
                };

            static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
            return Metadata;
        }

        // Allows MetaSound graph to interact with your node's inputs
        virtual FDataReferenceCollection GetInputs() const override
        {
            using namespace TutorialNodeNames;
            FDataReferenceCollection InputDataReferences;

            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InputAValue), InputA);
            InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InputBValue), InputB);

            return InputDataReferences;
        }

        // Allows MetaSound graph to interact with your node's outputs
        virtual FDataReferenceCollection GetOutputs() const override
        {
            using namespace TutorialNodeNames;
            FDataReferenceCollection OutputDataReferences;

            OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutputValue), TutorialNodeOutput);

            return OutputDataReferences;
        }

        // Used to instantiate a new runtime instance of your node
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors)
        {
            using namespace TutorialNodeNames;

            const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
            const FInputVertexInterface& InputInterface = DeclareVertexInterface().GetInputInterface();

            TDataReadReference<float> InputA = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface,
                METASOUND_GET_PARAM_NAME(InputAValue), InParams.OperatorSettings);
            TDataReadReference<float> InputB = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface,
                METASOUND_GET_PARAM_NAME(InputBValue), InParams.OperatorSettings);

            return MakeUnique<FTutorialOperator>(InParams.OperatorSettings, InputA, InputB);
        }

        // Primary node functionality
        void Execute()
        {
            *TutorialNodeOutput = *InputA + *InputB;
        }

    private:

        // Inputs
        FFloatReadRef InputA;
        FFloatReadRef InputB;

        // Outputs
        FFloatWriteRef TutorialNodeOutput;

    };

    class FTutorialNode : public FNodeFacade
    {
    public:
        FTutorialNode(const FNodeInitData& InitData) : FNodeFacade(InitData.InstanceName, InitData.InstanceID,
            TFacadeOperatorClass<FTutorialOperator>())
        {
        }
    };

    // Register node
    METASOUND_REGISTER_NODE(FTutorialNode);
}

#undef LOCTEXT_NAMESPACE
/////////////////////////////////////////////////////////////////////
// MetaCsound.build: Csound node for quadraphonic audio
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

#include "TCsoundOperator.h"
#include "MetasoundExecutableOperator.h"     // TExecutableOperator class
#include "MetasoundPrimitives.h"             // ReadRef and WriteRef descriptions for bool, int32, float, and string
#include "MetasoundNodeRegistrationMacro.h"  // METASOUND_LOCTEXT and METASOUND_REGISTER_NODE macros
#include "MetasoundFacade.h"				 // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "Containers/Array.h"

#define LOCTEXT_NAMESPACE "MetaCsound_FCsoundNode4"

namespace MetaCsound
{

    class METACSOUND_API FCsoundOperator4 : public TCsoundOperator<FCsoundOperator4>
    {
    public:
        // TODO: create struct that has all of the arguments, to avoid such long constructors?
        FCsoundOperator4(const FOperatorSettings& InSettings,
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
            return { TEXT("MetaCsound"), TEXT("Csound4"), TEXT("Audio") };
        }

        static const FText GetDisplayName()
        {
            return LOCTEXT("NodeDisplayName", "Csound (4)");
        }

        static const FText GetDescription()
        {
            return LOCTEXT("NodeDesc", "Csound performance with quadraphonic input and output.");
        }

        static constexpr int32 NumAudioChannelsIn = 4;
        static constexpr int32 NumAudioChannelsOut = 4;
        static constexpr int32 NumControlChannelsIn = 8;
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
    METASOUND_REGISTER_NODE(FCsoundNode4); // WIP Node registration using module startup/shutdown?

}

#undef LOCTEXT_NAMESPACE

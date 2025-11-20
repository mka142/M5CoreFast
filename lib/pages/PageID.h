#pragma once

// Enum for page identification
// Add more as needed for your app

enum PageID
{

    // PAGE SET
    BEFORE_CONCERT,
    BEFORE_CONCERT__SPONSORS, // Sponsor logos sub-page
    BEFORE_CONCERT__RESEARCH_FORM,    // Research form (sub-page of BEFORE_CONCERT)
    //
    APP_GUIDE,
    SLIDER_DEMO,
    SLIDER_DEMO__MEASUREMENT,         // Tension measurement demo (sub-page of SLIDER_DEMO)
    CONCERT_START,
    PIECE_ANNOUNCEMENT,
    PIECE_LISTENING,
    TENSION_MEASUREMENT,
    OVATION,
    // PAGE SET
    END_OF_CONCERT,
    END_OF_CONCERT__FEEDBACK_FORM,    // Feedback form (sub-page of END_OF_CONCERT)
    END_OF_CONCERT__FORM_FINISHED,    // Thank you page after feedback form
    //SPONSORS,         // Sponsor logos carousel 
    SPONSORS,         // Sponsor logos carousel
    //
    CHARGING,         // Internal charging screen (auto-displayed when charging)
    // Add more as needed
    LOADING // Special page for loading state
};

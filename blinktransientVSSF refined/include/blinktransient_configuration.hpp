//
// Copyright (c) 2017-2020 Santini Designs. All rights reserved.
//

#pragma once

#include <basic/types.hpp>

#include <eye/protocol.hpp>

namespace user_tasks::blinktransient {

/**
 * The welcome banner configuration class is configuration class for the welcome banner task.
 */
struct BlinkTransientConfiguration : public eye::protocol::EyerisTaskConfiguration
{
    using ptr_t = std::shared_ptr<BlinkTransientConfiguration>;  ///< Pointer type definition for the class

    /**
     * @brief Static factory.
     *
     * @param[in] other Pointer to another JSON object that defines the initial schema of this one.
     *
     * @return Pointer to a new class instance
     */
    [[maybe_unused]]
    static ptr_t factory_ptr(const basic::types::JSON::sptr_t& other)
    {
        return std::make_shared<BlinkTransientConfiguration>(other);
    }

    /**
     * @brief Constructor that instantiate the configuration from the prototype, but then let set the values
     * also based on the parent configuration.
     *
     * @param[in] other Pointer to a JsonObject used to initialize the configuration
     */
    explicit BlinkTransientConfiguration(basic::types::JSON::sptr_t other) :
        EyerisTaskConfiguration(other)
    {
        initializeSimulation(false);        // whether control task where a simulated blink happens in half trials
        initializeSubject("sbj");
        
        // gabor
        initializeSpFreq1(1);                // low spatial frequency (cycles/deg)
        initializeSpFreq2(2);
        initializeSpFreq3(3);
        initializeSpFreq4(4);
        initializeSpFreq5(5);
        initializeSpFreq6(6);
        initializeSpFreq7(7);
        initializeSpFreq8(8);          
        initializeGaborWidth(1920);         // pixels
        initializeGaborHeight(1080);         // pixels
        initializeGaborStd(-180);           // pixels
        initializeGaborIntensityLow(0.4);      // fixed intensity level (0~255); if < 0, then use pest
        initializeGaborIntensityHigh(0.4);      // fixed intensity level (0~255); if < 0, then use pest

        // PEST
        initializeInitAmp1(0.6);             // initial amplitude of the gabor ( 0 ~ bgnLuminance ); minus means no PEST adopted
        initializeInitAmp2(0.6);
        initializeInitAmp3(0.6);
        initializeInitAmp4(0.6);
        initializeInitAmp5(0.6);
        initializeInitAmp6(0.6);
        initializeInitAmp7(0.6);
        initializeInitAmp8(0.6);             // initial amplitude of the gabor ( 0 ~ bgnLuminance ); minus means no PEST adopted
        
        initializeInitStep1(1);                // low spatial frequency (cycles/deg)
        initializeInitStep2(2);
        initializeInitStep3(3);
        initializeInitStep4(4);
        initializeInitStep5(5);
        initializeInitStep6(6);
        initializeInitStep7(7);
        initializeInitStep8(8);         // threshold for convergence   // 0.75
        initializeWaldConst(1);
        initializeLog(false);
        initializeFixedStep(true);

        // timing
        initializeStartWaitDur(1000);
        initializeFixDur(1000);
        initializeFixDurVar(100);           // +-100 (ms)
        initializeGapDur(0);                // ms
        initializeRampDur(1500);            // ms
        initializeNPlateauDurs(1);
        initializePlateauDur(1000);         // ms
        initializePlateauDur2(250);         // ms
        initializePlateauDur3(1000);        // ms
        initializeMaskDur(1000);            // ms
        initializeResponseDur(1000);        // ms
        initializeBreakDur(500);            // ms
        initializeAbortDur(500);            // ms
        initializeItiDur(300);                // ms

        // beep
        initializeBeepHas2Tones(false);
        initializeBeepDur(50);             // ms
        initializeBeepFreq(4000);           // Hz
        initializeBeepTime1(-800);           // 150        //relative to fp off // relative to fp on
        initializeBeepTime2(-900);           // -350// -250//   -400 //-350//-200//    // relative to plateau on

        // other parameters
        initializeFpWidth(0.05);            // degrees
        initializeFpColorB(0);
        initializeFpColorG(0);
        initializeFpColorR(0);
        initializeFixWinR(0.5);             // fixation window radius (degrees)
        initializeFixWinVisible(true);
        initializeFixCheck(false);
        initializeBreakColorB(255);        // 00FF00
        initializeBreakColorG(0);        // 00FF00
        initializeBreakColorR(0);        // 00FF00
        initializeAbortColorB(0);     // FF0000
        initializeAbortColorG(0);     // FF0000
        initializeAbortColorR(255);     // FF0000
        initializeBgnLuminance(128);        // background luminance
        initializeScreenW(1920);            // 2560//1920//
        initializeScreenH(1080);             // 1440//1080//
        initializeScreenR(200);             // 200//
        initializeScreenID("AUS ROG PG259QNR");
        initializeGammaR(2.13);             // contrast & brightness = 0;   2.03 for Acer XB272 brightness: 40%     //1.9825
        initializeGammaG(2.13);             // contrast & brightness = 0;   2.03 for Acer XB272  brightness: 40%     //2.0376
        initializeGammaB(2.13);             // contrast & brightness = 0;   2.03 for Acer XB272  brightness: 40%     //2.2010
        initializeRed2Blue(2.85);           // red to blue ratio
        initializeGreen2Blue(10.94);        // green to blue ratio
        initializeNTrials(50);
        initializeRecalNTrials(100);
    }

    LC_PROPERTY_BOOL(IS_SIMULATION, "Is_Simulation", Simulation)
    LC_PROPERTY_STRING(SUBJECT, "Subject_Name", Subject)

    // gabor
    LC_PROPERTY_FLOAT(GABOR_SP_FREQ_1, "Gabor_Spatial_Freq_1", SpFreq1)
    LC_PROPERTY_FLOAT(GABOR_SP_FREQ_2, "Gabor_Spatial_Freq_2", SpFreq2)
    LC_PROPERTY_FLOAT(GABOR_SP_FREQ_3, "Gabor_Spatial_Freq_3", SpFreq3)
    LC_PROPERTY_FLOAT(GABOR_SP_FREQ_4, "Gabor_Spatial_Freq_4", SpFreq4)
    LC_PROPERTY_FLOAT(GABOR_SP_FREQ_5, "Gabor_Spatial_Freq_5", SpFreq5)
    LC_PROPERTY_FLOAT(GABOR_SP_FREQ_6, "Gabor_Spatial_Freq_6", SpFreq6)
    LC_PROPERTY_FLOAT(GABOR_SP_FREQ_7, "Gabor_Spatial_Freq_7", SpFreq7)
    LC_PROPERTY_FLOAT(GABOR_SP_FREQ_8, "Gabor_Spatial_Freq_8", SpFreq8)
    LC_PROPERTY_INT(GABOR_WIDTH, "Gabor_Width", GaborWidth)
    LC_PROPERTY_INT(GABOR_HEIGHT, "Gabor_HEIGHT", GaborHeight)
    LC_PROPERTY_INT(GABOR_STD, "Gabor_Std", GaborStd)
    LC_PROPERTY_FLOAT(GABOR_INTENSITY_LOW, "Gabor_Intensity_Low", GaborIntensityLow)
    LC_PROPERTY_FLOAT(GABOR_INTENSITY_HIGH, "Gabor_Intensity_High", GaborIntensityHigh)

    // PEST
    LC_PROPERTY_FLOAT(PEST_INI_TAMP_LOW, "Pest_Init_Amp_1", InitAmp1)                  // initial amplitude of the gabor ( 0 ~ bgnLuminance ); minus means no PEST adopted
    LC_PROPERTY_FLOAT(PEST_INI_TAMP_HIGH, "Pest_Init_Amp_2", InitAmp2)
    LC_PROPERTY_FLOAT(PEST_INI_TAMP_HIGH, "Pest_Init_Amp_3", InitAmp3)
    LC_PROPERTY_FLOAT(PEST_INI_TAMP_HIGH, "Pest_Init_Amp_4", InitAmp4) 
    LC_PROPERTY_FLOAT(PEST_INI_TAMP_HIGH, "Pest_Init_Amp_5", InitAmp5)
    LC_PROPERTY_FLOAT(PEST_INI_TAMP_HIGH, "Pest_Init_Amp_6", InitAmp6)
    LC_PROPERTY_FLOAT(PEST_INI_TAMP_HIGH, "Pest_Init_Amp_7", InitAmp7)
    LC_PROPERTY_FLOAT(PEST_INI_TAMP_HIGH, "Pest_Init_Amp_8", InitAmp8)                 // initial amplitude of the gabor ( 0 ~ bgnLuminance ); minus means no PEST adopted
    LC_PROPERTY_FLOAT(PEST_INIT_STEP_1, "Pest_Init_Step_1", InitStep1)               // 0.1// 0.02// 0.1// 0.01// 0.2 //0.5
    LC_PROPERTY_FLOAT(PEST_INIT_STEP_2, "Pest_Init_Step_2", InitStep2) 
    LC_PROPERTY_FLOAT(PEST_INIT_STEP_3, "Pest_Init_Step_3", InitStep3)               // 0.1// 0.02// 0.1// 0.01// 0.2 //0.5
    LC_PROPERTY_FLOAT(PEST_INIT_STEP_4, "Pest_Init_Step_4", InitStep4) 
    LC_PROPERTY_FLOAT(PEST_INIT_STEP_5, "Pest_Init_Step_5", InitStep5)
    LC_PROPERTY_FLOAT(PEST_INIT_STEP_6, "Pest_Init_Step_6", InitStep6)
    LC_PROPERTY_FLOAT(PEST_INIT_STEP_7, "Pest_Init_Step_7", InitStep7)
    LC_PROPERTY_FLOAT(PEST_INIT_STEP_8, "Pest_Init_Step_8", InitStep8)            
    LC_PROPERTY_FLOAT(PEST_WALD_CONST, "Pest_Wald_Const", WaldConst)
    LC_PROPERTY_BOOL(PEST_IS_LOG, "Pest_Is_Log", Log)
    LC_PROPERTY_BOOL(PEST_IS_FIXED_STEP, "Pest_Is_Fixed_Step", FixedStep)

    // timing
    LC_PROPERTY_INT(TIMING_START_WAIT_DUR, "Timing_Start_Wait_Dur", StartWaitDur)
    LC_PROPERTY_INT(TIMING_FIX_DUR, "Timing_Fix_Dur", FixDur)
    LC_PROPERTY_INT(TIMING_FIX_DUR_VAR, "Timing_Fix_Dur_Var", FixDurVar)            // +-100 (ms)
    LC_PROPERTY_INT(TIMING_GAP_DUR, "Timing_Gap_Dur", GapDur)                       // ms
    LC_PROPERTY_INT(TIMING_RAMP_DUR, "Timing_Ramp_Dur", RampDur)                    // ms
    LC_PROPERTY_INT(TIMING_N_PLATEAU_DURS, "Timing_N_Plateau_Durs", NPlateauDurs)
    LC_PROPERTY_INT(TIMING_PLATEAU_DUR, "Timing_Plateau_Dur", PlateauDur)           // ms
    LC_PROPERTY_INT(TIMING_PLATEAU_DUR2, "Timing_Plateau_Dur2", PlateauDur2)        // ms
    LC_PROPERTY_INT(TIMING_PLATEAU_DUR3, "Timing_Plateau_Dur3", PlateauDur3)        // ms
    LC_PROPERTY_INT(TIMING_MASK_DUR, "Timing_Mask_Dur", MaskDur)                    // ms
    LC_PROPERTY_INT(TIMING_RESPONSE_DUR, "Timing_Response_Dur", ResponseDur)        // ms
    LC_PROPERTY_INT(TIMING_BREAK_DUR, "Timing_Break_Dur", BreakDur)                 // ms
    LC_PROPERTY_INT(TIMING_ABORT_DUR, "Timing_Abort_Dur", AbortDur)                 // ms
    LC_PROPERTY_INT(TIMING_ITI_DUR, "Timing_Iti_Dur", ItiDur)                       // ms

    // beep
    LC_PROPERTY_BOOL(BEEP_HAS_2_TONES, "Beep_Has_2_Tones", BeepHas2Tones)
    LC_PROPERTY_INT(BEEP_DUR, "Beep_Dur", BeepDur)                                  // ms
    LC_PROPERTY_INT(BEEP_FREQ, "Beep_Freq", BeepFreq)                               // Hz
    LC_PROPERTY_INT(BEEP_TIME1, "Beep_Time1", BeepTime1)                            // 150        //relative to fp off // relative to fp on
    LC_PROPERTY_INT(BEEP_TIME2, "Beep_Time2", BeepTime2)                            // -350// -250//   -400 //-350//-200//    // relative to plateau on

    // other parameters
    LC_PROPERTY_FLOAT(FIX_FP_WIDTH, "Fix_FpWidth", FpWidth)            // degrees
    LC_PROPERTY_INT(FIX_FP_COLOR_B, "Fix_Fp_Color_B", FpColorB)
    LC_PROPERTY_INT(FIX_FP_COLOR_G, "Fix_Fp_Color_G", FpColorG)
    LC_PROPERTY_INT(FIX_FP_COLOR_R, "Fix_Fp_Color_R", FpColorR)
    LC_PROPERTY_FLOAT(Fix_WIN_R, "Fix_Win_Radius", FixWinR)             // fixation window radius (degrees)
    LC_PROPERTY_BOOL(FIX_WIN_VISIBLE, "Fix_Win_Visible", FixWinVisible)
    LC_PROPERTY_BOOL(FIX_CHECK, "Fix_Check", FixCheck)
    LC_PROPERTY_INT(FIX_BREAK_COLOR_B, "Fix_Break_Color_B", BreakColorB)        // 00FF00
    LC_PROPERTY_INT(FIX_BREAK_COLOR_G, "Fix_Break_Color_G", BreakColorG)        // 00FF00
    LC_PROPERTY_INT(FIX_BREAK_COLOR_R, "Fix_Break_Color_R", BreakColorR)        // 00FF00
    LC_PROPERTY_INT(FIX_ABORT_COLOR_B, "Fix_Abort_Color_B", AbortColorB)     // FF0000
    LC_PROPERTY_INT(FIX_ABORT_COLOR_G, "Fix_Abort_Color_G", AbortColorG)     // FF0000
    LC_PROPERTY_INT(FIX_ABORT_COLOR_R, "Fix_Abort_Color_R", AbortColorR)     // FF0000
    LC_PROPERTY_INT(BGN_LUMINANCE, "Bgn_Luminance", BgnLuminance)        // background luminance
    LC_PROPERTY_INT(SCREEN_W, "Screen_W", ScreenW)            // 2560//1920//
    LC_PROPERTY_INT(SCREEN_H, "Screen_H", ScreenH)             // 1440//1080//
    LC_PROPERTY_INT(SCREEN_R, "Screen_R", ScreenR)             // 200//
    LC_PROPERTY_STRING(SCREEN_ID, "Screen_ID", ScreenID)           // ASUS278
    LC_PROPERTY_FLOAT(SCREEN_GAMMA_R, "Screen_Gamma_R", GammaR)             // brightness: 40%     //1.9825
    LC_PROPERTY_FLOAT(SCREEN_GAMMA_G, "Screen_Gamma_G", GammaG)             // brightness: 40%     //2.0376
    LC_PROPERTY_FLOAT(SCREEN_GAMMA_B, "Screen_Gamma_B", GammaB)             // brightness: 40%     //2.2010
    LC_PROPERTY_FLOAT(SCREEN_RED_TO_BLUE, "Screen_Red_To_Blue", Red2Blue)
    LC_PROPERTY_FLOAT(SCREEN_GREEN_TO_BLUE, "Screen_Green_To_Blue", Green2Blue)
    LC_PROPERTY_INT(N_TRIALS, "N_Trials", NTrials)
    LC_PROPERTY_INT(Recal_N_Trials, "N_Trials_Recalibration", RecalNTrials)


};

}  // namespace user_tasks::blinktransient

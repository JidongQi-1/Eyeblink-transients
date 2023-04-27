#include <stdio.h>

#include "blinktransient.hpp"

#include <eye/configuration.hpp>
#include <eye/messaging.hpp>

#ifndef LINUX
#define LINUX
#endif

#ifdef WINDOWS
#include <direct.h>
int ACCESS(const char* path)
{
    return _access(path, 0);
}
void MKDIR(const char* path)
{
    _mkdir(path);
}
#else
#include <sys/stat.h>
#include <unistd.h>
int ACCESS(const char* path)
{
    return access(path, 0);
}
void MKDIR(const char* path)
{
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
#endif

using namespace std;

namespace user_tasks::blinktransient {

EYERIS_PLUGIN(BlinkTransient)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public methods

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlinkTransient::BlinkTransient() :
    EyerisTask()
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::eventCommand(int command, const basic::types::JSONView& arguments)
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::eventConfiguration(const BlinkTransientConfiguration::ptr_t& configuration)
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::eventConsoleChange(const basic::types::JSONView& change)
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::finalize()
{
    // called twice!!!    
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::initialize()
{
    srand(time(NULL));

    fixConfirmDur = 20; // ms
    hDrift = vDrift = 0;

    // setVideoMode(eye::graphics::VideoMode(getConfiguration()->getScreenW(), getConfiguration()->getScreenH(), getConfiguration()->getScreenR()));

    nTrials = getConfiguration()->getNTrials();
    curTrial = 0;
    curTrialLow = 0;
    nCorrects = 0;
    nErrors = 0;

    // re-calibration
    reCalibrator.nTrials = getConfiguration()->getRecalNTrials();
    reCalibrator.xOffset = reCalibrator.yOffset = 0;
    reCalibrator.isFinished = true;
    reCalibrator.pTarget = newImagePlane("images/whitecross.tga");
    reCalibrator.pTracker = newImagePlane("images/redcross.tga");
    reCalibrator.pTarget->setPosition(0,0);
    reCalibrator.pTracker->setPosition(0,0);
    reCalibrator.pTarget->hide();
    reCalibrator.pTracker->hide();
    moveToFront(reCalibrator.pTracker);


    // fixation window
    FixWindow.xPix = FixWindow.yPix = 0;
    FixWindow.rPix = getAngleConverter()->arcmin2PixelH(getConfiguration()->getFixWinR() * 60);
    FixWindow.visible = getConfiguration()->isFixWinVisible();
    FixWindow.isCheck = getConfiguration()->isFixCheck();
    FixWindow.pStim = newImagePlane("images/arcs.tga");
    FixWindow.pStim->enableTransparency(true);
    FixWindow.pStim->setSize(FixWindow.rPix*1.05*2+1, FixWindow.rPix*1.05*2+1);   // *1.05 to make the inner edge the boundary of the window
    FixWindow.pStim->hide();

    // fixation point
    FP.color[0] = getConfiguration()->getFpColorR();
    FP.color[1] = getConfiguration()->getFpColorG();;
    FP.color[2] = getConfiguration()->getFpColorB();;
    FP.breakColor[0] = getConfiguration()->getBreakColorR();;
    FP.breakColor[1] = getConfiguration()->getBreakColorG();;
    FP.breakColor[2] = getConfiguration()->getBreakColorB();;
    FP.abortColor[0] = getConfiguration()->getAbortColorR();;
    FP.abortColor[1] = getConfiguration()->getAbortColorG();;
    FP.abortColor[2] = getConfiguration()->getAbortColorB();;
    FP.xPix = FP.yPix = 0;
    FP.wPix = getAngleConverter()->arcmin2PixelH(getConfiguration()->getFpWidth() * 60);
    FP.duration = getConfiguration()->getFixDur();
    FP.pStim = newSolidPlane(FP.wPix, FP.wPix, eye::graphics::RGB(FP.color[0], FP.color[1], FP.color[2]));
    FP.pStim->setPosition(FP.xPix, FP.yPix);
    FP.pStim->hide();

    // Gabor
    Gabor.rampDur = getConfiguration()->getRampDur();
    Gabor.plateauDur = getConfiguration()->getPlateauDur();
    Gabor.xPix = Gabor.yPix = 0;
    Gabor.wPix = getConfiguration()->getGaborWidth();
    Gabor.hPix = getConfiguration()->getGaborHeight();
    Gabor.spFreq = getConfiguration()->getSpFreq1();
    Gabor.std = getConfiguration()->getGaborStd();
    Gabor.pShader = std::make_shared<eye::graphics::TexturedSurfaceShader>();
    Gabor.pShader->load(
            fromAssetsManager()->inAssets("shaders/shader_main.vert"),
            fromAssetsManager()->inAssets("shaders/shader_grating_noisyBitStealing.frag") );
    Gabor.pShader->setUniform("screenOffsetX", getConfiguration()->getScreenW()/2.0f);
    Gabor.pShader->setUniform("screenOffsetY", getConfiguration()->getScreenH()/2.0f);
    Gabor.pShader->setUniform("cX", 0.0f);
    Gabor.pShader->setUniform("cY", 0.0f);
    Gabor.pShader->setUniform("RB", getConfiguration()->getRed2Blue());
    Gabor.pShader->setUniform("GB", getConfiguration()->getGreen2Blue());
    Gabor.pShader->setUniform("rndseed1", (float)rand());
    Gabor.pShader->setUniform("rndseed2", (float)rand());
    Gabor.pShader->setUniform("transparency", 1.0f);
    Gabor.pShader->setUniform("spatialFreq", Gabor.spFreq);
    Gabor.pShader->setUniform("degPerPix", getAngleConverter()->pixel2ArcminH(1) / 60.0f);
    Gabor.pShader->dismiss();
    Gabor.pStim = newTexturedPlane(getConfiguration()->getGaborWidth(), getConfiguration()->getGaborHeight(), std::vector<eye::graphics::RGB>(getConfiguration()->getGaborWidth() * getConfiguration()->getGaborHeight(), eye::graphics::RGB::BLACK), Gabor.pShader);
    
    // gabor contrast
    contrasts.fixedIntensities[0] = getConfiguration()->getGaborIntensityLow();
    contrasts.fixedIntensities[1] = getConfiguration()->getGaborIntensityHigh();
    if (contrasts.fixedIntensities[0] < 0) {
        // create destination folder
        string folder = "../../dev_ws/user_tasks/blink_transient/data/" + getConfiguration()->getSubject() + "/";
        for (size_t index = folder.find("/"); index != std::string::npos; index = folder.find("/", index + 1))
            if (ACCESS(folder.substr(0, index).c_str()) == -1) MKDIR(folder.substr(0, index).c_str());
        
        bgnLuminance = getConfiguration()->getBgnLuminance();

        if(ACCESS((folder + "pestLow").c_str()) == -1) MKDIR((folder + "pestLow").c_str());
        if (ACCESS((folder + "pestLow/pest.txt").c_str()) == -1)
            contrasts.pests[0] = Pest(getConfiguration()->getInitAmpLow() > bgnLuminance ? bgnLuminance : getConfiguration()->getInitAmpLow(), getConfiguration()->getTargetRatio(),
                getConfiguration()->getInitStepLow(), getConfiguration()->getWaldConst(), getConfiguration()->isLog(), getConfiguration()->isFixedStep());
        else contrasts.pests[0] = Pest((folder + "pestLow").c_str());

        if(ACCESS((folder + "pestHigh").c_str()) == -1) MKDIR((folder + "pestHigh").c_str());
        if (ACCESS((folder + "pestHigh/pest.txt").c_str()) == -1)
            contrasts.pests[1] = Pest(getConfiguration()->getInitAmpHigh() > bgnLuminance ? bgnLuminance : getConfiguration()->getInitAmpHigh(), getConfiguration()->getTargetRatio(),
                getConfiguration()->getInitStepHigh(), getConfiguration()->getWaldConst(), getConfiguration()->isLog(), getConfiguration()->isFixedStep());
        else contrasts.pests[1] = Pest((folder + "pestHigh").c_str());
    }
    
    // mask
    Mask.xPix = Mask.yPix = 0;
    Mask.wPix = Gabor.wPix;
    Mask.hPix = Gabor.hPix;
    Mask.pStim = newImagePlane("images/masks/mask_00.bmp");
    Mask.pStim->setPosition(Mask.xPix, Mask.yPix);
    Mask.pStim->setSize(Mask.wPix, Mask.hPix);
    Mask.pStim->hide();

    moveToFront(FP.pStim);
    moveToFront(FixWindow.pStim);

    pBeeper = new MyBeeper(&TmrTrial);

    VCardRefreshFrame.iFrame = -1;
    VCardRefreshFrame.state = S_MAIN_FINAL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::setup()
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::streamAnalog(const eye::signal::DataSliceAnalogBlock::ptr_t& data)
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::streamDigital(const eye::signal::DataSliceDigitalBlock::ptr_t& data)
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::streamEye(const eye::signal::DataSliceEyeBlock::ptr_t& data)
{
    if(!isInitialized()) return;

    static int iFrame = -1;
    iFrame++;

    if(iFrame == 0){
        // setBackgroundColor(eye::graphics::RGB(bgnLuminance, bgnLuminance, bgnLuminance));
        Transition(SC_MAIN, S_MAIN_INIT, 0);
        Transition(SC_BEEP, S_BB_WAIT, 0);
    }

    if( iFrame == VCardRefreshFrame.iFrame + 1 )
        switch( VCardRefreshFrame.state ){
            case S_MAIN_START:
                tTrialStart = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
                TmrSMain.start(chrono::milliseconds(getConfiguration()->getStartWaitDur()));    // restart the timer for the main state chain
                break;

            case S_MAIN_RAMP_ON:
                tRampOn = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
                break;

            case S_MAIN_PLATEAU_ON:
                tPlateauOn = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
                break;

            case S_MAIN_MASK_ON:
                tMaskOn = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
                break;

            case S_MAIN_RESPONSE:
                tMaskOff = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
                break;
        }

    storeUserStream( "FrameTime", (float)TmrTrial.getTime<basic::time::microseconds_t>().count()/1000.0 );
    storeUserStream( "FrameIndex", (float)iFrame );

    storeUserStream( "getDataframe()", data->getDataframe() );
    storeUserStream( "size()", (int)data->size() );

    storeUserStream( "last_x", (float)(data->getLatest()->calibrated1.x() - reCalibrator.xOffset) );
    storeUserStream( "last_y", (float)(data->getLatest()->calibrated1.y() - reCalibrator.yOffset) );
    storeUserStream( "last_dataframeNumber", data->getLatest()->dataframeNumber );
    storeUserStream( "last_sliceNumber", data->getLatest()->sliceNumber );
    storeUserStream( "last_timestamp", (float)data->getLatest()->timestamp.elapsed().count() );
    storeUserStream( "last_totalNumberOfSlices", (double)data->getLatest()->statistics.totalNumberOfSlices );
    storeUserStream( "last_slicesPerSecond", data->getLatest()->statistics.slicesPerSecond );

    storeUserStream( "first_x", (float)(data->getFirst()->calibrated1.x() - reCalibrator.xOffset) );
    storeUserStream( "first_y", (float)(data->getFirst()->calibrated1.y() - reCalibrator.yOffset) );
    storeUserStream( "first_dataframeNumber", data->getFirst()->dataframeNumber );
    storeUserStream( "first_sliceNumber", data->getFirst()->sliceNumber );
    storeUserStream( "first_timestamp", (float)data->getFirst()->timestamp.elapsed().count() );
    storeUserStream( "first_totalNumberOfSlices", (double)data->getFirst()->statistics.totalNumberOfSlices );
    storeUserStream( "first_slicesPerSecond", data->getFirst()->statistics.slicesPerSecond );

    switch(CurState[SC_MAIN]){

        case S_MAIN_INIT:
            if (TmrTrial.hasExpired()) Transition( SC_MAIN, S_MAIN_START, iFrame );
            break;

        case S_MAIN_START:
            if( EyeWindowCheck(data->getLatest()) ) Transition( SC_MAIN, S_MAIN_CONFIRM_FIX, iFrame );
            else if( TmrSMain.hasExpired() ) Transition( SC_MAIN, S_MAIN_ABORT, iFrame );
            break;

        case S_MAIN_CONFIRM_FIX:
            if( !EyeWindowCheck(data->getLatest()) ) Transition( SC_MAIN, S_MAIN_START, iFrame );
            else if( TmrSMain.hasExpired() ) Transition( SC_MAIN, S_MAIN_FP_ON, iFrame );
            break;

        case S_MAIN_FP_ON:
            if (getConfiguration()->isFixCheck() && !EyeWindowCheck(data->getLatest())) Transition(SC_MAIN, S_MAIN_BREAK, iFrame);
            else if( TmrSMain.hasExpired() ) Transition( SC_MAIN, S_MAIN_RAMP_ON, iFrame );
            break;

        case S_MAIN_GAP_ON:
            if(getConfiguration()->isFixCheck() && !EyeWindowCheck(data->getLatest()) ) Transition( SC_MAIN, S_MAIN_BREAK, iFrame );
            else if( TmrSMain.hasExpired() ) Transition( SC_MAIN, S_MAIN_RAMP_ON, iFrame );
            break;
        
        case S_MAIN_RAMP_ON:
            if (getConfiguration()->isFixCheck() && !EyeWindowCheck(data->getLatest())) Transition(SC_MAIN, S_MAIN_BREAK, iFrame);
            UpdateGabor();
            if( TmrSMain.hasExpired() ) Transition( SC_MAIN, S_MAIN_PLATEAU_ON, iFrame );
            break;

        case S_MAIN_PLATEAU_ON:
            if (getConfiguration()->isFixCheck() && !EyeWindowCheck(data->getLatest())) Transition(SC_MAIN, S_MAIN_BREAK, iFrame);
            if( TmrSMain.hasExpired() ) Transition( SC_MAIN, S_MAIN_MASK_ON, iFrame );
            break;

        case S_MAIN_MASK_ON:
            if( TmrSMain.hasExpired() ) Transition( SC_MAIN, S_MAIN_RESPONSE, iFrame );
            break;

        case S_MAIN_RESPONSE:
            if( isResponsed )
                Transition( SC_MAIN, S_MAIN_FINAL, iFrame );
            else if( TmrSMain.hasExpired() )
                Transition( SC_MAIN, S_MAIN_ABORT, iFrame );
            break;

        case S_MAIN_FINAL:
            if( curTrial < nTrials ) Transition( SC_MAIN, S_MAIN_INTERVAL, iFrame );
            break;

        case S_MAIN_INTERVAL:
            if( TmrSMain.hasExpired() ){
                SaveData();
                if(!(curTrial % getConfiguration()->getRecalNTrials())) Transition( SC_MAIN, S_MAIN_CALIBRATION, iFrame );
                else Transition( SC_MAIN, S_MAIN_START, iFrame );
            }
            break;

        case S_MAIN_CALIBRATION:
            UpdateReCalibrator(data->getLatest());
            if(reCalibrator.isFinished) Transition( SC_MAIN, S_MAIN_START, iFrame );
            break;

        case S_MAIN_ABORT:
            if( TmrSMain.hasExpired() )
                Transition( SC_MAIN, S_MAIN_FINAL, iFrame );
            break;

        case S_MAIN_BREAK:
            if( TmrSMain.hasExpired() )
                Transition( SC_MAIN, S_MAIN_FINAL, iFrame );
            break;
    }

    switch(CurState[SC_BEEP]){
        case S_BB_WAIT:
            break;
        case S_BB_DELAY:
            if( TmrSBeep.hasExpired() ) Transition( SC_BEEP, S_BB_ON, iFrame);
            break;
        case S_BB_ON:
            Transition( SC_BEEP, S_BB_WAIT, iFrame );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::streamKeyboard(const eye::signal::DataSliceKeyboardBlock::ptr_t& data)
{
    auto keyboard = data->getLatest();
    if (!isResponsed && CurState[SC_MAIN] == S_MAIN_MASK_ON || CurState[SC_MAIN] == S_MAIN_RESPONSE)
        if (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::keyboard_keys_e::KEY_LEFT)) { // left arrow
            Response = abs(Gabor.orientation);  // counter-clockwise
            isResponsed = true;
            tResponse = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
            tBlinkBeepOn = pBeeper->GetBeepTime();
            pBeeper->Play(1000, getConfiguration()->getBeepDur());
        }
        else if (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::keyboard_keys_e::KEY_RIGHT)) { // right arrow
            Response = -abs(Gabor.orientation); // clockwise
            isResponsed = true;
            tResponse = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
            tBlinkBeepOn = pBeeper->GetBeepTime();
            pBeeper->Play(1000, getConfiguration()->getBeepDur());
        }

    if( CurState[SC_MAIN] == S_MAIN_CALIBRATION ){
        reCalibrator.xOffset += keyboard->isKeyPressed(source_keyboard::keyboard_keys_e::keyboard_keys_e::KEY_LEFT) - keyboard->isKeyPressed(source_keyboard::keyboard_keys_e::keyboard_keys_e::KEY_RIGHT);  // left - right
        reCalibrator.yOffset += keyboard->isKeyPressed(source_keyboard::keyboard_keys_e::keyboard_keys_e::KEY_DOWN) - keyboard->isKeyPressed(source_keyboard::keyboard_keys_e::keyboard_keys_e::KEY_UP);  // down - up
        reCalibrator.isFinished = keyboard->isKeyPressed(source_keyboard::keyboard_keys_e::keyboard_keys_e::KEY_RETURN) || keyboard->isKeyPressed(source_keyboard::keyboard_keys_e::keyboard_keys_e::KEY_NUMPAD_ENTER);   // Enter
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::streamJoypad(const eye::signal::DataSliceJoypadBlock::ptr_t& data)
{
    auto joypad = data->getLatest();
    //if( !isResponsed && CurState[SC_MAIN] == S_MAIN_RAMP_ON || CurState[SC_MAIN] == S_MAIN_PLATEAU_ON || CurState[SC_MAIN] == S_MAIN_MASK_ON || CurState[SC_MAIN] == S_MAIN_RESPONSE )    // allow report during stimulus
    if (!isResponsed && CurState[SC_MAIN] == S_MAIN_MASK_ON || CurState[SC_MAIN] == S_MAIN_RESPONSE)    // do not allow report during stimulus
        if( joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_L1) ){
            Response = abs(Gabor.orientation);  // counter-clockwise
            isResponsed = true;
            tResponse = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
            tBlinkBeepOn = pBeeper->GetBeepTime();
            pBeeper->Play(1000, getConfiguration()->getBeepDur());
        }else if( joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_R1) ){
            Response = -abs(Gabor.orientation); // clockwise
            isResponsed = true;
            tResponse = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
            tBlinkBeepOn = pBeeper->GetBeepTime();
            pBeeper->Play(1000, getConfiguration()->getBeepDur());
        }

    if( CurState[SC_MAIN] == S_MAIN_CALIBRATION )
    {
        reCalibrator.xOffset += joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_LEFT) - joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_RIGHT);
        reCalibrator.yOffset += joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_DOWN) - joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_UP);
    
        if( joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_R1) ) reCalibrator.isFinished = true;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::streamMonitor(const eye::signal::DataSliceMonitorBlock::ptr_t& data)
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::streamMouse(const eye::signal::DataSliceMouseBlock::ptr_t& data)
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::streamVideoCard(const eye::signal::DataSliceVideoCardBlock::ptr_t& data)
{
    // Nothing to do
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlinkTransient::teardown()
{
    if(pBeeper) delete pBeeper;

    if(contrasts.fixedIntensities[0] < 0){
        string folder = "../../dev_ws/user_tasks/blink_transient/data/" + getConfiguration()->getSubject() + "/pestLow";
        contrasts.pests[0].saveInstance(folder.c_str());
        folder = "../../dev_ws/user_tasks/blink_transient/data/" + getConfiguration()->getSubject() + "/pestHigh";
        contrasts.pests[1].saveInstance(folder.c_str());
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// state functions
void BlinkTransient::SMInitStuff(unsigned int)
{
    startTrial();
    TmrTrial.start(2000ms);
    storeUserVariable("RecordStartTime", TmrTrial.getTime<basic::time::microseconds_t>().count()/1000.0);
}


void BlinkTransient::SMStartStuff(unsigned int iFrame)
{
#ifdef BlinkTransient_DEBUG
    cout << "\tSMStartStuff" << endl;
#endif

    // TmrTrial.start(chrono::milliseconds(getConfiguration()->getStartWaitDur() + getConfiguration()->getFixDur() + getConfiguration()->getRampDur() +
    //    getConfiguration()->getPlateauDur() + getConfiguration()->getMaskDur() + getConfiguration()->getResponseDur() + 1000));

    tTrialStart = -1;
    tFpOn = -1;
    tGapOn = -1;
    tRampOn = -1;
    tPlateauOn = -1;
    tMaskOn = -1;
    tMaskOff = -1;
    tResponse = -1;
    tAbort = -1;
    tBreak = -1;
    tBlinkBeepOn = -1;
    Response = 0;   // no response
    isResponsed = false;

    reCalibrator.pTarget->hide();
    reCalibrator.pTracker->hide();

    Gabor.spFreq = rand() * 1.0 / RAND_MAX < (nTrials/8.0 - curTrial) / (nTrials - curTrial) || (contrasts.fixedIntensities[0] < 0) 
                   getConfiguration()->getSpFreq1() : getConfiguration()->getSpFreq2(): getConfiguration()->getSpFreq3(): 
                   getConfiguration()->getSpFreq4(): getConfiguration()->getSpFreq5(): getConfiguration()->getSpFreq6():
                   getConfiguration()->getSpFreq7(): getConfiguration()->getSpFreq8();
                
    if (contrasts.fixedIntensities[0] < 0) Gabor.amplitude = contrasts.curLevel = contrasts.pests[Gabor.spFreq > (getConfiguration()->getSpFreq1()
    +getConfiguration()->getSpFreq2()+getConfiguration()->getSpFreq3()+getConfiguration()->getSpFreq4()+getConfiguration()->getSpFreq5()+getConfiguration()->getSpFreq6()
    +getConfiguration()->getSpFreq7()+getConfiguration()->getSpFreq8())/8].GetCurLevel();
    else Gabor.amplitude = contrasts.fixedIntensities[Gabor.spFreq > (getConfiguration()->getSpFreq1()
    +getConfiguration()->getSpFreq2()+getConfiguration()->getSpFreq3()+getConfiguration()->getSpFreq4()+getConfiguration()->getSpFreq5()+getConfiguration()->getSpFreq6()
    +getConfiguration()->getSpFreq7()+getConfiguration()->getSpFreq8())/8];

    Gabor.orientation = rand() % 2 ? -45 : 45;
    Gabor.phase = (rand() % 4) * 90;// 360;
    Gabor.pShader->setUniform("spatialFreq", Gabor.spFreq);
    Gabor.pShader->setUniform("phase", Gabor.phase);
    Gabor.pShader->setUniform("orientation", Gabor.orientation / 180.0f * 3.14159f);
    Gabor.pShader->dismiss();
    
    int nPlateauDurs = getConfiguration()->getNPlateauDurs();
    if (nPlateauDurs == 1)
        Gabor.plateauDur = getConfiguration()->getPlateauDur();
    else if(nPlateauDurs == 2)
        Gabor.plateauDur = rand() / double(RAND_MAX) > 0.5 ? getConfiguration()->getPlateauDur() : getConfiguration()->getPlateauDur2();
    else {
        float rnd = rand() % 3;
        if (rnd == 0) Gabor.plateauDur = getConfiguration()->getPlateauDur();
        else if (rnd == 1) Gabor.plateauDur = getConfiguration()->getPlateauDur2();
        else Gabor.plateauDur = getConfiguration()->getPlateauDur3();
    }

    // mask
    char str[256] = { 0 };
    sprintf(str, "images/masks/mask_%02d.bmp", rand() % 50);
    Mask.pStim->applyTexture(eye::graphics::TextureFactory::load(fromAssetsManager()->inAssets(str)));
    Mask.pStim->setPosition(Mask.xPix, Mask.yPix);
    Mask.pStim->setSize(Mask.wPix, Mask.hPix);


    FP.pStim->setColor(eye::graphics::RGB(FP.color[0], FP.color[1], FP.color[2]));
    FP.pStim->setSize(FP.wPix, FP.wPix);
    if (FixWindow.visible) FixWindow.pStim->show();
    VCardRefreshFrame.iFrame = iFrame;
    VCardRefreshFrame.state = S_MAIN_START;
}

void BlinkTransient::SMConfirmFixStuff(unsigned int)
{
    TmrSMain.start(chrono::milliseconds(fixConfirmDur));
}

void BlinkTransient::SMFpOnStuff(unsigned int iFrame)
{
    FP.duration = getConfiguration()->getFixDur() + ( rand() / (double)RAND_MAX * 2 - 1 ) * getConfiguration()->getFixDurVar();
    if( FP.duration < fixConfirmDur ) FP.duration = fixConfirmDur;
    TmrSMain.start(chrono::milliseconds(FP.duration - fixConfirmDur));

    curTrial++;
    if(Gabor.spFreq > (getConfiguration()->getSpFreq1()
    +getConfiguration()->getSpFreq2()+getConfiguration()->getSpFreq3()+getConfiguration()->getSpFreq4()+getConfiguration()->getSpFreq5()+getConfiguration()->getSpFreq6()
    +getConfiguration()->getSpFreq7()+getConfiguration()->getSpFreq8())/8) curTrialLow++;
    tFpOn = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0 - fixConfirmDur;

    Transition( SC_BEEP, S_BB_DELAY, iFrame );
}

void BlinkTransient::SMGapOnStuff(unsigned int iFrame)
{
    tGapOn = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
    TmrSMain.start(chrono::milliseconds(getConfiguration()->getGapDur()));
    FP.pStim->hide();
    FixWindow.pStim->hide();
}

void BlinkTransient::SMRampOnStuff(unsigned int iFrame)
{
    TmrSMain.start(chrono::milliseconds(Gabor.rampDur));
    Gabor.timer.start(chrono::milliseconds(Gabor.rampDur + Gabor.plateauDur));
    
    //FP.pStim->hide();     // 2018.08.16
    //FixWindow.pStim->hide();

    Gabor.pStim->show();

    VCardRefreshFrame.iFrame = iFrame;
    VCardRefreshFrame.state = S_MAIN_RAMP_ON;
}

void BlinkTransient::SMPlateauOnStuff(unsigned int iFrame)
{
    TmrSMain.start(chrono::milliseconds(Gabor.plateauDur));

    UpdateGabor();
    VCardRefreshFrame.iFrame = iFrame;
    VCardRefreshFrame.state = S_MAIN_PLATEAU_ON;
}

void BlinkTransient::SMMaskOnStuff(unsigned int iFrame)
{
    TmrSMain.start(chrono::milliseconds(getConfiguration()->getMaskDur()));

    Gabor.pStim->hide();
    FP.pStim->hide();       // 2018.08.16
    FixWindow.pStim->hide();

    Mask.pStim->show();
    VCardRefreshFrame.iFrame = iFrame;
    VCardRefreshFrame.state = S_MAIN_MASK_ON;
}

void BlinkTransient::SMResponseStuff(unsigned int iFrame)
{
    TmrSMain.start(chrono::milliseconds(getConfiguration()->getResponseDur()));

    Mask.pStim->hide();

    VCardRefreshFrame.iFrame = iFrame;
    VCardRefreshFrame.state = S_MAIN_RESPONSE;
}

void BlinkTransient::SMFinalStuff(unsigned int iFrame)
{
    hideAllObjects();
    bool stepped = false;

    if( contrasts.fixedIntensities[0] < 0 ){
        SaveData();
        endTrial();
        storeUserVariable("RecordEndTime", TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0);
    }
}

void BlinkTransient::SMCalibrationStuff(unsigned int iFrame)
{
    reCalibrator.isFinished = false;
    reCalibrator.pTarget->show();
    reCalibrator.pTracker->show();
}

void BlinkTransient::SMIntervalStuff(unsigned int iFrame)
{
#ifdef BlinkTransient_DEBUG
    cout << "\tSMIntervalStuff" << endl;
#endif
    TmrSMain.start(chrono::milliseconds(getConfiguration()->getItiDur()));
}

void BlinkTransient::SMAbortStuff(unsigned int iFrame)
{
    tAbort = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
    //Beep(1000,700);
    //Beep(750,700);
    Transition(SC_BEEP, S_BB_WAIT, iFrame);
    TmrSMain.start(chrono::milliseconds(getConfiguration()->getAbortDur()));
    hideAllObjects();
    if( tFpOn < 0 ) FP.pStim->setColor(eye::graphics::RGB(FP.breakColor[0], FP.breakColor[1], FP.breakColor[2]));
    else FP.pStim->setColor(eye::graphics::RGB(FP.abortColor[0], FP.abortColor[1], FP.abortColor[2]));
    FP.pStim->setSize(FP.wPix * 3, FP.wPix * 3);
    FP.pStim->show();
}

void BlinkTransient::SMBreakStuff(unsigned int iFrame)
{
    tBreak = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
    //Beep(1000,700);
    //Beep(750,700);
    Transition(SC_BEEP, S_BB_WAIT, iFrame);
    TmrSMain.start(chrono::milliseconds(getConfiguration()->getBreakDur()));
    hideAllObjects();
    FP.pStim->setColor(eye::graphics::RGB(FP.breakColor[0], FP.breakColor[1], FP.breakColor[2]));
    FP.pStim->setSize(FP.wPix * 3, FP.wPix * 3);
    FP.pStim->show();
}

void BlinkTransient::SBWaitStuff(unsigned int){}

void BlinkTransient::SBDelayStuff(unsigned int iFrame)
{
    double rnd = rand() / (double)RAND_MAX;
    
    /***2018.11.20***/
    if (rnd <= 0.5)
        TmrSBeep.start(chrono::milliseconds(FP.duration - fixConfirmDur + getConfiguration()->getBeepTime1()));  // relative to ramp onset
    else
        TmrSBeep.start(chrono::milliseconds(FP.duration - fixConfirmDur + Gabor.rampDur + getConfiguration()->getBeepTime2()));   // relative to plateau on
    
    // frequency
    // if ( !getConfiguration()->getHas2Tones() || rnd < 0.5 / 3 || 1.0 / 3 <= rnd && rnd < 0.5 || 2.0 / 3 <= rnd && rnd < 2.5 / 3)
    //     beepFreq = getConfiguration()->getBeepFreq() + 1000;
    // else
    //     beepFreq = getConfiguration()->getBeepFreq() - 1000;


}

void BlinkTransient::SBOnStuff(unsigned int iFrame)
{
    TmrSBeep.stop();
     /*tBlinkBeepOn = TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0;
     Beep( getConfiguration()->getBeepFreq(), getConfiguration()->getBeepDur() );*/
    pBeeper->Play(getConfiguration()->getBeepFreq(), getConfiguration()->getBeepDur());

}

void BlinkTransient::Transition( STATECHAIN sSet, STATE s, unsigned int iFrame )
{
    CurState[sSet] = s;
    (this->*(this->TransitionFuncs[s])) (iFrame);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// other functions
bool BlinkTransient::EyeWindowCheck(shared_ptr<eye::signal::DataSliceEye> pSlice)
{
    if(!FixWindow.isCheck) return true;
#ifdef BlinkTransient_DEBUG
    return true;
#endif
    float x = getAngleConverter()->arcmin2PixelH(pSlice->calibrated1.x() - hDrift);
    float y = getAngleConverter()->arcmin2PixelV(pSlice->calibrated1.y() - vDrift);
    return pow(x - FixWindow.xPix, 2) + pow(y - FixWindow.yPix, 2) <= pow(FixWindow.rPix, 2);
}

void BlinkTransient::UpdateGabor()
{
    if(Gabor.timer.getTime<basic::time::microseconds_t>().count() / 1000.0 <= Gabor.rampDur){
        Gabor.pShader->setUniform("contrast", float(Gabor.amplitude / 128.0f * Gabor.timer.getTime<basic::time::microseconds_t>().count() / 1000.0 / Gabor.rampDur));
        Gabor.pShader->dismiss();
    }
}

void BlinkTransient::UpdateReCalibrator(shared_ptr<eye::signal::DataSliceEye> pSlice)
{
    if(CurState[SC_MAIN] == S_MAIN_CALIBRATION && !reCalibrator.isFinished){
        reCalibrator.pTarget->show();
        float x, y;
        x = getAngleConverter()->arcmin2PixelH(pSlice->calibrated1.x() - reCalibrator.xOffset);
        y = getAngleConverter()->arcmin2PixelV(pSlice->calibrated1.y() - reCalibrator.yOffset);
        reCalibrator.pTracker->setPosition(x, y);
        reCalibrator.pTracker->show();

    } else{
        reCalibrator.pTarget->hide();
        reCalibrator.pTracker->hide();
    }
}

void BlinkTransient::SaveData()
{
    char trialType = 'u';   // unknown
    if( tBreak > 0 ) trialType = 'b';   // error
    else if( tAbort > 0 ) trialType = 'a';  // abort
    else if( Response == Gabor.orientation ) {
        trialType = 'c';    // correct;
        nCorrects++;
    }
    else if( Response != Gabor.orientation ){
        trialType = 'e';    // error
        nErrors++;
    }
    
    printf("curTrial: %d\n", curTrial);
    if( tBreak >= 0 ) std::cout << "BREAK!!!" << std::endl;
    else{
        if( tAbort < 0 || tAbort > 0 && tMaskOff > 0 ){
            printf("gaborSpFreq: %.1f\n", Gabor.spFreq);
            printf("gaborAmp: %.2f\n", Gabor.amplitude);
            printf("gaborPhase: %.1f\n", Gabor.phase);
            printf("gaborOri: %.1f\n", Gabor.orientation);
        }
        if( tAbort >= 0 ) printf("ABORT!!!\n");
        else printf("Response: %.1f\n", Response);
    }
    printf("PlateauDur: %d\n", Gabor.plateauDur);
    printf("Performance: %.1f%%\n", 100. * nCorrects / (nCorrects+nErrors));
    printf("Trial duration: %.1f\n", TmrTrial.getTime<basic::time::microseconds_t>().count() / 1000.0);
    
    // General Settings
    storeUserVariable("expName", "BlinkTransient");
    storeUserVariable("sbjName", getConfiguration()->getSubject());
    storeUserVariable("screenR", getVideoMode().refreshRate);
    storeUserVariable("screenWPix", getVideoMode().width);
    storeUserVariable("screenHPix", getVideoMode().height);
    storeUserVariable("screenID", getConfiguration()->getScreenID());
    storeUserVariable("gammaR", getConfiguration()->getGammaR());
    storeUserVariable("gammaG", getConfiguration()->getGammaG());
    storeUserVariable("gammaB", getConfiguration()->getGammaB());
    storeUserVariable("Red2Blue", getConfiguration()->getRed2Blue());
    storeUserVariable("Green2Blue", getConfiguration()->getGreen2Blue());
    float w=0, h=0, d=0;
    getAngleConverter()->getScreenSizeMm(w,h);
    d = getAngleConverter()->getEyetrackerDistance();
    storeUserVariable("screenWmm", w);
    storeUserVariable("screenHmm", h);
    storeUserVariable("screenDmm", d); // distance
    storeUserVariable("bgLuminance", bgnLuminance);

    // fixation
    storeUserVariable("fpWidth", getConfiguration()->getFpWidth());
    storeUserVariable("fpColor", (getConfiguration()->getFpColorR() << 16) + (getConfiguration()->getFpColorG() << 8) + getConfiguration()->getFpColorB());
    storeUserVariable("breakColor", (getConfiguration()->getBreakColorR() << 16) + (getConfiguration()->getBreakColorG() << 8) + getConfiguration()->getBreakColorB());
    storeUserVariable("abortColor", (getConfiguration()->getAbortColorR() << 16) + (getConfiguration()->getAbortColorG() << 8) + getConfiguration()->getAbortColorB());
    storeUserVariable("fixWinR", getConfiguration()->getFixWinR());
    storeUserVariable("fixWinVisible", getConfiguration()->isFixWinVisible());
    storeUserVariable("fixCheck", getConfiguration()->isFixCheck());

    // beep
    storeUserVariable("beepFreq", getConfiguration()->getBeepFreq());
    storeUserVariable("beepDur", getConfiguration()->getBeepDur());
    
    // trial variables
    auto trialVars = basic::types::JSON();

    // Timing
    trialVars["iTrial"] = curTrial;
    trialVars["trialType"] = trialType;
    trialVars["tTrialStart"] = tTrialStart;
    trialVars["tFpOn"] = tFpOn;
    trialVars["tGapOn"] = tGapOn;
    trialVars["tRampOn"] = tRampOn;
    trialVars["tPlateauOn"] = tPlateauOn;
    trialVars["tMaskOn"] = tMaskOn;
    trialVars["tMaskOff"] = tMaskOff;
    switch(trialType){
        case 'b':
            trialVars["tResponse"] = tBreak;
            break;
        case 'a':
            trialVars["tResponse"] = tAbort;
            break;
        default:
            trialVars["tResponse"] = tResponse;
    }
    trialVars["response"] = Response;
    trialVars["tBlinkBeepOn"] = tBlinkBeepOn;

    // gabor
    trialVars["gaborSpFreq"] = Gabor.spFreq;   // cycles/degree
    trialVars["gaborOri"] = Gabor.orientation;  // degrees
    trialVars["gaborPhase"] = Gabor.phase;      // degrees
    trialVars["gaborStdPix"] = Gabor.std;
    trialVars["gaborWPix"] = Gabor.wPix;
    trialVars["gaborHPix"] = Gabor.hPix;
    trialVars["gaborAmp"] = Gabor.amplitude;    // 0 ~ bgnLuminance

    storeUserEvent(trialVars);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected methods

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private methods

}  // namespace user_tasks::blinktransient

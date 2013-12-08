// ratecontrol.h
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RATECONTROL_H
#define RATECONTROL_H

#include <QObject>

#include "configobject.h"
#include "controlobject.h"
#include "engine/enginecontrol.h"
#include "engine/enginesync.h"

const int RATE_TEMP_STEP = 500;
const int RATE_TEMP_STEP_SMALL = RATE_TEMP_STEP * 10.;
const int RATE_SENSITIVITY_MIN = 100;
const int RATE_SENSITIVITY_MAX = 2500;
const double TRACK_POSITION_MASTER_HANDOFF = 0.98;

class BpmControl;
class Rotary;
class ControlTTRotary;
class ControlObject;
class ControlPotmeter;
class ControlPushButton;
class EngineChannel;
class PositionScratchController;
#ifdef __VINYLCONTROL__
class VinylControlControl;
#endif

// RateControl is an EngineControl that is in charge of managing the rate of
// playback of a given channel of audio in the Mixxx engine. Using input from
// various controls, RateControl will calculate the current rate.
class RateControl : public EngineControl {
    Q_OBJECT
public:
    RateControl(const char* _group, ConfigObject<ConfigValue>* _config, EngineSync* enginesync);
    virtual ~RateControl();

    void setBpmControl(BpmControl* bpmcontrol);
    void setEngineChannel(EngineChannel* pChannel);
#ifdef __VINYLCONTROL__
    void setVinylControlControl(VinylControlControl* vinylcontrolcontrol);
#endif
    // Must be called during each callback of the audio thread so that
    // RateControl has a chance to update itself.
    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int bufferSamples);
    // Returns the current engine rate.
    double calculateRate(double baserate, bool paused, int iSamplesPerBuffer, bool* isScratching);
    double getRawRate() const;
    ControlObject* getBeatDistanceControl();
    double getMode() const;
    void setMode(double state);
    double getFileBpm() const { return m_pFileBpm ? m_pFileBpm->get() : 0.0; }
    EngineChannel* getChannel() { Q_ASSERT(m_pChannel); return m_pChannel; }
    const QString getGroup() const { return m_sGroup; }


    // Set rate change when temp rate button is pressed
    static void setTemp(double v);
    // Set rate change when temp rate small button is pressed
    static void setTempSmall(double v);
    // Set rate change when perm rate button is pressed
    static void setPerm(double v);
    // Set rate change when perm rate small button is pressed
    static void setPermSmall(double v);
    /** Set Rate Ramp Mode */
    static void setRateRamp(bool);
    /** Set Rate Ramp Sensitivity */
    static void setRateRampSensitivity(int);
    virtual void notifySeek(double dNewPlaypos);
    void checkTrackPosition(double fractionalPlaypos);

  public slots:
    void slotControlRatePermDown(double);
    void slotControlRatePermDownSmall(double);
    void slotControlRatePermUp(double);
    void slotControlRatePermUpSmall(double);
    void slotControlRateTempDown(double);
    void slotControlRateTempDownSmall(double);
    void slotControlRateTempUp(double);
    void slotControlRateTempUpSmall(double);
    void slotControlFastForward(double);
    void slotControlFastBack(double);
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  private slots:
    void slotControlPlay(double);
    void slotSyncModeChanged(double);
    void slotSyncMasterChanged(double);
    void slotSyncSlaveChanged(double);
    void slotRateSliderChanged(double);

  private:
    double getJogFactor() const;
    double getWheelFactor() const;

    /** Set rate change of the temporary pitch rate */
    void setRateTemp(double v);
    /** Add a value to the temporary pitch rate */
    void addRateTemp(double v);
    /** Subtract a value from the temporary pitch rate */
    void subRateTemp(double v);
    /** Reset the temporary pitch rate */
    void resetRateTemp(void);
    /** Get the 'Raw' Temp Rate */
    double getTempRate(void);

    /** Values used when temp and perm rate buttons are pressed */
    static double m_dTemp, m_dTempSmall, m_dPerm, m_dPermSmall;

    QString m_sGroup;
    EngineChannel* m_pChannel;
    EngineSync* m_pEngineSync;
    ControlPushButton *buttonRateTempDown, *buttonRateTempDownSmall,
        *buttonRateTempUp, *buttonRateTempUpSmall;
    ControlPushButton *buttonRatePermDown, *buttonRatePermDownSmall,
        *buttonRatePermUp, *buttonRatePermUpSmall;
    ControlObject *m_pRateDir, *m_pRateRange, *m_pRateEngine;
    ControlObject* m_pBeatDistance;
    ControlPotmeter* m_pRateSlider;
    ControlPotmeter* m_pRateSearch;
    ControlPushButton* m_pReverseButton;
    ControlObject* m_pBackButton;
    ControlObject* m_pForwardButton;
    ControlObject* m_pPlayButton;

    ControlTTRotary* m_pWheel;
    ControlTTRotary* m_pScratch;
    ControlTTRotary* m_pOldScratch;
    PositionScratchController* m_pScratchController;

    ControlPushButton* m_pScratchToggle;
    ControlObject* m_pJog;
    ControlObject* m_pVCEnabled;
    ControlObject* m_pVCScratching;
#ifdef __VINYLCONTROL__
    VinylControlControl *m_pVinylControlControl;
#endif
    Rotary* m_pJogFilter;

    ControlObject *m_pSampleRate;

    TrackPointer m_pTrack;

    // For Master Sync
    BpmControl* m_pBpmControl;
    ControlObject* m_pSyncMode;

    // The current loaded file's detected BPM
    ControlObject* m_pFileBpm;

    // Enumerations which hold the state of the pitchbend buttons.
    // These enumerations can be used like a bitmask.
    enum RATERAMP_DIRECTION {
        RATERAMP_NONE = 0,  // No buttons are held down
        RATERAMP_DOWN = 1,  // Down button is being held
        RATERAMP_UP = 2,    // Up button is being held
        RATERAMP_BOTH = 3   // Both buttons are being held down
    };

    // Rate ramping mode:
    //  RATERAMP_STEP: pitch takes a temporary step up/down a certain amount.
    //  RATERAMP_LINEAR: pitch moves up/down in a progresively linear fashion.
    enum RATERAMP_MODE {
        RATERAMP_STEP = 0,
        RATERAMP_LINEAR = 1
    };

    // This defines how the rate returns to normal. Currently unused.
    // Rate ramp back mode:
    //  RATERAMP_RAMPBACK_NONE: returns back to normal all at once.
    //  RATERAMP_RAMPBACK_SPEED: moves back in a linearly progresive manner.
    //  RATERAMP_RAMPBACK_PERIOD: returns to normal within a period of time.
    enum RATERAMP_RAMPBACK_MODE {
        RATERAMP_RAMPBACK_NONE,
        RATERAMP_RAMPBACK_SPEED,
        RATERAMP_RAMPBACK_PERIOD
    };

    // The current rate ramping direction. Only holds the last button pressed.
    int m_ePbCurrent;
    //  The rate ramping buttons which are currently being pressed.
    int m_ePbPressed;

    // This is true if we've already started to ramp the rate
    bool m_bTempStarted;
    // Set to the rate change used for rate temp
    double m_dTempRateChange;
    // Set the Temporary Rate Change Mode
    static enum RATERAMP_MODE m_eRateRampMode;
    // The Rate Temp Sensitivity, the higher it is the slower it gets
    static int m_iRateRampSensitivity;
    // Temporary pitchrate, added to the permanent rate for calculateRate
    double m_dRateTemp;
    // Previously-known bpm value, used for determining if sync speed has actually changed.
    double m_dOldBpm;
    enum RATERAMP_RAMPBACK_MODE m_eRampBackMode;
    // Return speed for temporary rate change
    double m_dRateTempRampbackChange;
};

#endif /* RATECONTROL_H */

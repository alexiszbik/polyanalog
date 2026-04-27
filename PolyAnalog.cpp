#include "daisy_seed.h"
#include "daisysp.h"

#include "DaisyYMNK/DaisyYMNK.h"
#include "DaisyYMNK/QSPI/PresetManager.h"
#include "Source/PolyAnalogCore.h"

using namespace daisy;
using namespace daisysp;
using namespace ydaisy;

DaisySeed hw;
PolyAnalogCore polyAnalog;
DaisyBase db = DaisyBase(&hw, &polyAnalog);

PresetManager pm;
DisplayManager *display = DisplayManager::GetInstance();

OneShotMs screenResetAfterSave;

void PolyAnalogRefreshScreen()
{
    polyAnalog.updateScreen();
}

void OnWillNeedToResetScreen()
{
    screenResetAfterSave.Start(1000, PolyAnalogRefreshScreen);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    db.process(out, size);
}

void InitHID()
{
    db.listen();
}

void UpdateValues() {
    polyAnalog.displayValuesOnScreen();
}

int main(void)
{
    db.init(AudioCallback, 48);

    display->Init(&hw);
    display->WriteNow("YMNK", "PolyAnalog Synth");

    pm.Init(&hw);

    db.setDisplayManager(display);
    db.setPresetManager(&pm);

    //polyFM.setHIDValue(PolyFMCore::MidiLed, 1);

    BlockingAction blocker;
    blocker.Run(1000, InitHID); //Wait for HID to init
    //Is it the best solution ? Maybe ?

    polyAnalog.ready();
    polyAnalog.setWillNeedToResetScreenHandler(OnWillNeedToResetScreen);

    //EveryMs displayValuesUpdater (125, UpdateValues);

    for(;;)
    {
        db.listen();
        screenResetAfterSave.Update();
        //displayValuesUpdater.Update();
        display->Update();
    }
    
}

# IIS Lab 03 Writeup

[![Demo video](https://img.youtube.com/vi/CoE66TLl6M0/0.jpg)](https://youtu.be/CoE66TLl6M0)

For this lab I decided to focus on lock picking haptics. I was inspired by games like Skyrim for their lock picking mechanics, but thought it would be cool to do a "real" version of it. A friend also got locked out of their house recently, which led me to find a cheap lock pick set I bought a while ago and barely used.

Aside from the h-bridge (with two vibration motors), I added a force sensitive resistor and 3 touch pins (with aluminum foil plates/contact pads).

To unlock the lock, the user must apply an "enaging" amount of force to the tension wrench (the force sensitive resistor) while simultaneously determining the correct order of the pins to push.

There are a number of feedback "events" that trigger haptic feedback for the user. Here they are grouped by which vibration motor they trigger:

**Pick:**
PIN_CONTACT
PIN_ACK
FALSE_SET

**Wrench:**
TENSION_ENGAGED
OVER_TENSION
RELEASE_RESET

**Pick and Wrench:**
PIN_SET
UNLOCKED

Just upload this to your Esp32 and update the pin numbers in the code to match your wiring!

## AI Usage

I wrote the initial code with just GithubCopilot, but only with one vibration motor, one button, and the force sensitive resistor. I then asked AI to add the second vibration motor and the two buttons/pins (and debug logging). Then I had it refactor my pin state and feedback event code. While I don't love my AI usage in this project (I'm confident I could have implemented all of this myself), it (once again) allowed me to focus more on the physical interaction and haptics. I also used AI to write the "Shave and a haircut... two bits" haptic feedback sequence. Also used it to create the git repo and push the code (to be so honest just to see if it had the ability to).

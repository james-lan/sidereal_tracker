# Sidereal Star Tracker
A simple adjustable in the field star tracker for use with Stepper driven Equatorial mounts.

Should be usable with barn door trackers as well. (The gearing calculation isn't provided.) 

The instructions are in the Arudino .ino file. Step 1 and Step 2. 

Simplest way to use it is to get an EQ1 and print these parts https://www.thingiverse.com/thing:1033336 and then and Arduino Mega/Uno

Then you can use a RAMPS, GRBL CNC shield, or breadboard to set up the ciruit. 

I use Pololu DRV8825s (and some clones) modified for fast decay: https://www.pololu.com/product/2133 There is one important thing which is setting the current on those. The video under current limiting describes it well. (Usually I follow the procedure, of turn it to min, turn it on, then slowly increase it until it's not having issues, and leave it.) 

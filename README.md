# LightroomShuttlePro
Lightroom plugin for support contour design ShuttlePro v2

## PREREQUISITES
1. Have Shuttle Driver installed from: https://www.contourdesign.com/support/drivers/

## INSTALLATION

1. Put "ShuttlePlugin.lrplugin" directory to Lightroom plugin directory, by default it is: c:\Users\<UserName>\AppData\Roaming\Adobe\Lightroom\Modules\ShuttlePlugin.lrplugin\

## CONFIGURATION

Shuttle keys action assignment is done in [commandKeyToAction.xml](ShuttlePlugin.lrplugin/commandKeyToAction.xml) file.
Configuration supports multiple layouts. [commandKeyToAction.xml](ShuttlePlugin.lrplugin/commandKeyToAction.xml) provides two layouts by default: Library and Develop.
Switching between layouts is done by pressing the following key combination (configured by [commandKeyToAction.xml](ShuttlePlugin.lrplugin/commandKeyToAction.xml) file and could be adjusted, key numbers could be found on ShuttlePROv2_keys.jpg picture): 

Button_9+Button_7 -> Library

Button_9+Button_8 -> Develop

![Key numbers](./ShuttlePROv2_keys.jpg)

TBD: provide full description of commandKeyToAction.xml file format

## USAGE:

1. Start Lightroom
2. Use Shuttle as configured by commandKeyToAction.xml

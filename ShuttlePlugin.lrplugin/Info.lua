--[[----------------------------------------------------------------------------
ADOBE SYSTEMS INCORPORATED
Copyright 2016 Adobe Systems Incorporated
All Rights Reserved.
NOTICE: Adobe permits you to use, modify, and distribute this file in accordance
with the terms of the Adobe license agreement accompanying it. If you have received
this file from a source other than Adobe, then your use, modification, or distribution
of it requires the prior written permission of Adobe.
------------------------------------------------------------------------------]]
return {
  LrSdkVersion = 6.0,
  LrPluginName = "Shuttle Controller",
  LrToolkitIdentifier = 'com.abrilevskiy.Shuttle_Controller',
  LrInitPlugin = "Start.lua", -- runs when plug-in initializes (this is the main script)
  LrForceInitPlugin = true, -- initializes the plug-in automatically at startup.
  LrShutdownApp = "Shutdown.lua", -- tells the main script to exit and waits for it to finish.
  LrShutdownPlugin = "Shutdown.lua",
  LrDisablePlugin = "Stop.lua", -- tells the main script to exit.
  LrExportMenuItems = {
    {
      title = "Start",
      file = "Start.lua",
    },
    {
      title = "Stop",
      file = "Stop.lua",
    },
  },
  VERSION = { major=6, minor=9, revision=0, build=200000, },
}
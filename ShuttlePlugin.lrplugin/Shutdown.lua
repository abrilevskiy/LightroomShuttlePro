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
  LrShutdownFunction = function( doneFunction, progressFunction )
    local LrTasks = import "LrTasks"
    LrTasks.startAsyncTask( function()
        if _G.running then
          local LrDate = import "LrDate"
          local start = LrDate.currentTime()
          local estimatedWait = 0.5 -- seconds
          -- tell the run loop to exit
          _G.running = false
          -- wait for the run loop to exit, updating the
          -- progress bar to give the user feedback
          while not _G.shutdown do
            local now = LrDate.currentTime()
            local percent = (now - start) / estimatedWait
            percent = math.min( 1, math.max( 0, percent ) )
            progressFunction( percent )
            LrTasks.sleep( 0.1 ) -- seconds
          end
        end
        -- tell the app we're done and it’s safe to shut down
        progressFunction( 1 ) -- 100% complete
        doneFunction()
      end )
  end,
}
--[[----------------------------------------------------------------------------
ADOBE SYSTEMS INCORPORATED
Copyright 2016 Adobe Systems Incorporated
All Rights Reserved.
NOTICE: Adobe permits you to use, modify, and distribute this file in accordance
with the terms of the Adobe license agreement accompanying it. If you have received
this file from a source other than Adobe, then your use, modification, or distribution
of it requires the prior written permission of Adobe.
------------------------------------------------------------------------------]]
local LrDialogs = import "LrDialogs"
local LrFunctionContext = import "LrFunctionContext"
local LrTasks = import "LrTasks"
local LrApplication = import "LrApplication"
local LrApplicationView = import "LrApplicationView"
local LrSelection = import "LrSelection"
local LrDevelopController = import "LrDevelopController"
local LrSocket = import "LrSocket"
local LrTableUtils = import "LrTableUtils"
--local LrMobdebug = import 'LrMobdebug'
local LrShell = import 'LrShell'
local LrPathUtils = import 'LrPathUtils'
--LrMobdebug.start()
--==============================================================================

-- Port numbers
-- port zero indicates that we want the OS to auto-assign the port
local AUTO_PORT = 0
-- port number used to send change notifications
local sendPort = 56788
-- port number used to receive commands
local receivePort = 56789
--==============================================================================
-- All of the Develop parameters that we will monitor for changes. For a complete
-- listing of all parameter names, see the API documentation for LrDevelopController.
local develop_params = {
    "Temperature",
    "Tint",
    "Exposure",
    "Contrast",
    "Highlights",
    "Shadows",
    "Whites",
    "Blacks",
    "Clarity",
    "Vibrance",
    "Saturation",
}

local ActiveDevelopParam = "Temperature"


local adjustBasic = {
    "Temperature",
    "Tint",
    "Exposure",
    "Contrast",
    "Highlights",
    "Shadows",
    "Whites",
    "Blacks",
    "Clarity",
    "Vibrance",
    "Saturation",
}

local adjustDetails = {
    "Sharpness",
    "SharpenRadius",
    "SharpenDetail",
    "SharpenEdgeMasking",
    "LuminanceSmoothing",
    "LuminanceNoiseReductionDetail",
    "LuminanceNoiseReductionContrast",
    "ColorNoiseReduction",
    "ColorNoiseReductionDetail",
    "ColorNoiseReductionSmoothness",
}

local extensionCommand = {
    "SwitchAdjustParam"
}

local function extensionCommandProcessor(command, value)
    local adjustListToSerach = adjustBasic

    if command == "SwitchDetailsAdjustParam" then
        adjustListToSerach = adjustDetails
    end

    local index = nil
    for i = 1, #adjustListToSerach do
        if adjustListToSerach[i] == ActiveDevelopParam then
            index = i
            break
        end
    end
    if index == nil then
        ActiveDevelopParam = adjustListToSerach[1]

    else
        if (value == "+" and index == #adjustListToSerach) then
            ActiveDevelopParam = adjustListToSerach[1]
        elseif (value == "-" and index == 1) then
            ActiveDevelopParam = adjustListToSerach[#adjustListToSerach]
        else
            if (value == "+") then
                ActiveDevelopParam = adjustListToSerach[index + 1]
            elseif (value == "-") then
                ActiveDevelopParam = adjustListToSerach[index - 1]
                ActiveDevelopParam = adjustListToSerach[index - 1]
            end
        end
    end
    LrDialogs.showBezel(ActiveDevelopParam, 1)
end

local function switchDevelopParam(panelName, direction)
end

local develop_param_set = {}
for _, key in ipairs(develop_params) do
    develop_param_set[key] = true
end
--------------------------------------------------------------------------------
-- Checks to see if observer[ key ] is equal to the given value. If the value has
-- changed, reports the change to the given sender.
-- Used to notify external processes when settings change in Lr.
local function updateValue(observer, sender, key, value)
    if observer[key] ~= value then
        -- for table types, check if any values have changed
        if type(value) == "table" and type(observer[key]) == "table" then
            local different = false
            for k, v in pairs(value) do
                if observer[key][k] ~= v then
                    different = true
                    break
                end
            end
            for k, v in pairs(observer[key]) do
                if value[k] ~= v then
                    different = true
                    break
                end
            end
            if not different then
                return
            end
        end
        observer[key] = value
        local data = LrTableUtils.tableToString {
            key = key,
            value = value,
        }
        if WIN_ENV then
            data = string.gsub(data, "\n", "\r\n")
        end
        sender:send(data)
    end
end

--------------------------------------------------------------------------------
-- Given a key/value pair that has been parsed from a receiver port message, calls
-- the appropriate API to adjust a setting in Lr.
local function setValue(key, value)
    if key == "SwitchBasicAdjustParam" or key == "SwitchDetailsAdjustParam" then
        extensionCommandProcessor(key, value)
        return false
    end
    if key == "SwitchToModule" then
        LrApplicationView.switchToModule(value)
        return false
    end
    if value == "+" then -- ex: "Exposure = +"
        LrDevelopController.increment(key)
        return true
    end
    if value == "-" then -- ex: "Exposure = -"
        LrDevelopController.decrement(key)
        return true
    end
    if value == "reset" then -- ex: "Exposure = reset"
        LrDevelopController.resetToDefault(key)
        return true
    end
    if key == "label" then -- ex: "label = red"
        LrSelection.setColorLabel(value)
        return true
    end
    if key == "select" then -- ex: "select = next"
        if value == "next" then
            LrSelection.nextPhoto()
            return true
        elseif value == "previous" then
            LrSelection.previousPhoto()
            return true
        end
        return false
    end
    local numericValue = tonumber(value)
    if numericValue then
        if key == "rating" then -- ex: "rating = 3"
            LrSelection.setRating(numericValue)
            return true
        end
        if key == "flag" then -- ex: "flag = 1"
            if numericValue == -1 then
                LrSelection.flagAsReject()
                return true
            elseif numericValue == 0 then
                LrSelection.removeFlag()
                return true
            elseif numericValue == 1 then
                LrSelection.flagAsPick()
                return true
            end
            return false
        end
        if key and develop_param_set[key] then -- ex: "Exposure = 1.5"
            LrDevelopController.setValue(key, numericValue)
            return true
        end
    end
end

--------------------------------------------------------------------------------
-- Simple parser for messages sent from the external process over the socket
-- connection, formatted as "key = value". (ex: "rating = 2")
local function parseMessage(data)
    if type(data) == "string" then
        local _, _, key, value = string.find(data, "([^ ]+)%s*=%s*(.*)")
        if key == "ActiveDevelopParam" then
            key = ActiveDevelopParam
        end
        return key, value
    end
end

--------------------------------------------------------------------------------
-- checks all Develop parameters for any changes that happened in Lr, reporting
-- them to the sender socket.
local function updateDevelopParameters(observer)
    local sender = observer._sender
    for _, param in ipairs(develop_params) do
        updateValue(observer, sender, param, LrDevelopController.getValue(param))
    end
end

--------------------------------------------------------------------------------
local senderPort, senderConnected, senderObserver
local receiverPort, receiverConnected
--------------------------------------------------------------------------------
-- Called by both the send socket and the receive socket when they begin their
-- attempt to establish a connection to a port number.
local function maybeStartService()
    -- For the purpose of this demo, we are letting the OS select port numbers.
    -- So we will use a bezel message to tell the user what thse ports are so they
    -- can connect to them via Telnet (or similar) to send and receive messages.
    if senderPort and receiverPort then
        LrTasks.startAsyncTask(function()
            -- Give them 10 seconds to connect.
            for countDown = 10, 1, -1 do
                if not _G.running then
                    break
                end
                --if senderConnected and receiverConnected then
                if receiverConnected then
                    break
                end
                local msg = "Connect to port:"
                if not receiverConnected then
                    msg = string.format("%s\nReceiver = %d", msg, receiverPort)
                    LrShell.openFilesInApp( { _PLUGIN.path }, LrPathUtils.child(_PLUGIN.path ,  "ShuttleController.exe" ))
                end
                if not senderConnected then
                    msg = string.format("%s\nSender = %d", msg, senderPort)
                end
                msg = string.format("%s\n%d", msg, countDown)
                LrDialogs.showBezel(msg, 1)
                LrTasks.sleep(1)

            end
        end)
    end
end

--------------------------------------------------------------------------------
local function makeSenderSocket(context)
    -- A socket connection that sends messages from the plugin to the external process.
    local sender = LrSocket.bind {
        functionContext = context,
        address = "localhost",
        port = sendPort,
        mode = "send",
        plugin = _PLUGIN,
        onConnecting = function(socket, port)
            senderPort = port
            maybeStartService()
        end,
        onConnected = function(socket, port)
            senderConnected = true
        end,
        onMessage = function(socket, message)
            -- Nothing, we don't expect to get any messages back.
        end,
        onClosed = function(socket)
            -- If the other side of this socket is closed,
            -- tell the run loop below that it should exit.
            _G.running = false
        end,
        onError = function(socket, err)
            if err == "timeout" then
                socket:reconnect()
            end
        end,
    }
    -- This object is used to observe Develop parameter changes and
    -- report them all to the sender socket.
    senderObserver = {
        _sender = sender,
    }
    LrDevelopController.addAdjustmentChangeObserver(context, senderObserver, updateDevelopParameters)
    -- do initial update
    updateDevelopParameters(senderObserver)
    return sender
end

--------------------------------------------------------------------------------
local function makeReceiverSocket(context)
    -- A socket connection that receives messages from the external process and executes
    -- commands in Lightroom.
    local receiver = LrSocket.bind {
        functionContext = context,
        port = receivePort,
        mode = "receive",
        plugin = _PLUGIN,
        onConnecting = function(socket, port)
            receiverPort = port
            maybeStartService()
        end,
        onConnected = function(socket, port)
            receiverConnected = true
        end,
        onClosed = function(socket)
            -- If the other side of this socket is closed,
            -- tell the run loop below that it should exit.
            _G.running = false
        end,
        onMessage = function(socket, message)
            if type(message) == "string" then
                local key, value = parseMessage(message)
                if key and value then
                    if setValue(key, value) then
                        -- For the purpose of this demo, also show a bezel.
                        LrDialogs.showBezel(string.format("%s %s",
                            tostring(key),
                            tostring(value)), 4)
                    end
                end
            end
        end,
        onError = function(socket, err)
            if err == "timeout" then
                socket:reconnect()
            end
        end,
    }
    -- automatically scroll sliders into view whenever they are adjusted
    LrDevelopController.revealAdjustedControls(true)
    return receiver
end

--------------------------------------------------------------------------------
-- Start everything in an async task so we can sleep in a loop until we are shut down.
LrTasks.startAsyncTask(function()
    -- A function context is required for the socket API below. When this context
    -- is exited all socket connections that have been created from it will be
    -- closed. We stay inside this context indefinitiely by spinning in a sleep
    -- loop until told to exit.
    LrFunctionContext.callWithContext('socket_remote', function(context)
        local sender = makeSenderSocket( context )
        local receiver = makeReceiverSocket(context)
        LrDialogs.showBezel("Controller Demo Running")
        -- Loop until this plug-in global is set to false, which happens when the external process
        -- closes the socket connection(s), or if the user selects the menu command "File >
        -- Plug-in Extras > Stop" , or when Lightroom is shutting down.
        _G.running = true
        while _G.running do
            LrTasks.sleep(1 / 2) -- seconds
        end
        _G.shutdown = true
        if senderConnected then
            sender:close()
        end
        senderObserver = nil
        if receiverConnected then
            receiver:close()
        end
        LrDialogs.showBezel("Remote Connections Closed", 4)
    end)
end)

local MainScene = class("MainScene", cc.load("mvc").ViewBase)

local function getStringError(errorCode)
	local REUPLOAD_OK = 0
	local GDPS_LOGIN_ERROR = 1
	local GD_LOGIN_ERROR = 2
	local INVALID_LEVELID = 3
	local LEVELID_OTHER_ACCOUNT = 4
	local UPLOAD_LEVEL_ERROR = 5
	local UNKNOWN_SERVER_ERROR = 6
		
	local REUPLOAD_OK_STR = "Reupload Succsseful"
	local GDPS_LOGIN_ERROR_STR = "Invalid GDPS Login"
	local GD_LOGIN_ERROR_STR = "Invalid GD Login"
	local INVALID_LEVELID_STR = "Invalid Level ID"
	local LEVELID_OTHER_ACCOUNT_STR = "Level belongs to another account"
	local UPLOAD_LEVEL_ERROR_STR = "Error while uploading level"
	local UNKNOWN_SERVER_ERROR = "Unknown Server Error"
	
	if errorCode == REUPLOAD_OK then return REUPLOAD_OK_STR end
	if errorCode == GDPS_LOGIN_ERROR then return GDPS_LOGIN_ERROR_STR end
	if errorCode == GD_LOGIN_ERROR then return GD_LOGIN_ERROR_STR end
	if errorCode == INVALID_LEVELID then return INVALID_LEVELID_STR end
	if errorCode == LEVELID_OTHER_ACCOUNT then return LEVELID_OTHER_ACCOUNT_STR end
	if errorCode == UPLOAD_LEVEL_ERROR then return UPLOAD_LEVEL_ERROR_STR end
	if errorCode == UNKNOWN_SERVER_ERROR then return UNKNOWN_SERVER_ERROR_STR end
	
	return "Unknown Error"
end

function MainScene:onCreate()

	-- add HelloWorld label
	local gdpsUsername = ccui.TextField:create("GDPS Username", "", 30)
	gdpsUsername:move(display.cx - 300, display.cy + 150)
	gdpsUsername:setCursorEnabled(true)
	gdpsUsername:addTo(self)
	
	local gdpsPassword = ccui.TextField:create("GDPS Password", "", 30)
	gdpsPassword:move(display.cx - 300, display.cy + 100)
	gdpsPassword:setCursorEnabled(true)
	gdpsPassword:addTo(self)
	
	local gdUsername = ccui.TextField:create("GD Username", "", 30)
	gdUsername:move(display.cx + 300, display.cy + 150)
	gdUsername:setCursorEnabled(true)
	gdUsername:addTo(self)
	
	local gdPassword = ccui.TextField:create("GD Password", "", 30)
	gdPassword:move(display.cx + 300, display.cy + 100)
	gdPassword:setCursorEnabled(true)
	gdPassword:addTo(self)
	
	local levelIdText = ccui.TextField:create(" LevelID ", "", 30)
	levelIdText:move(display.cx, display.cy + 150)
	levelIdText:setCursorEnabled(true)
	levelIdText:addTo(self)
	
	local menu = cc.Menu:create()
	
	local label = cc.Label:createWithSystemFont("GDPS Editor Level Reuploader", "", 40)
	local menuItem = cc.MenuItemLabel:create(label)
	menuItem:move(0, 270)
	menuItem:addTo(menu)
	
	local label = cc.Label:createWithSystemFont("Reupload", "", 30)
	local menuItem = cc.MenuItemLabel:create(label)
	menuItem:move(0, 100)
	menuItem:addTo(menu)
	
	local resultLabel = cc.Label:createWithSystemFont("", "", 30)
	resultLabel:setTag(1945)
	resultLabel:move(display.cx, display.cy - 50)
	resultLabel:addTo(self)
	
	menu:addTo(self)
	
	--CALLBACKS (defined here to access labels)
	
	local function onReupload(sender)
		local str_gdpsUsername = gdpsUsername:getString()
		local str_gdpsPassword = gdpsPassword:getString()
		local str_gdUsername = gdUsername:getString()
		local str_gdPassword = gdPassword:getString()
		local levelID = levelIdText:getString()
		if levelID == nil or levelID == '' then
			resultLabel:setString("levelID is empty")
			return
		end
		resultLabel:setString("Reuploading...")
		resultLabel:setColor(cc.c3b(255, 255, 255))
		print(reupload_level(str_gdpsUsername, str_gdpsPassword, str_gdUsername, str_gdPassword, levelID))
	end
	menuItem:registerScriptTapHandler(onReupload)
	
	cc.exports.reupload_callback = function(result, err)
		
		if result > 10 then
			print(string.format("result: %s", result))
			resultLabel:setString(string.format("%s, levelID: %d", getStringError(0), result))
			resultLabel:setColor(cc.c3b(0, 255, 0))
		else
			local errorStr = string.format("result: %d, error: %s", result, err)
			print(errorStr)
			resultLabel:setString(string.format("%s, info: %s", getStringError(result), err))
			resultLabel:setColor(cc.c3b(255, 0, 0))
		end
	end
end

return MainScene

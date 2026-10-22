-- Uninstall Reckless Sudoku.app: the program taken off this Mac, the installer's work undone and nothing of the other
-- Reckless products touched. Compiled by packaging/macos/make_installer.sh with osacompile, uninstall.sh beside it in
-- its Resources, signed as the app is and installed with it in /Applications/Reckless Technology. It asks first, then
-- asks for an administrator's password once - the licence and notices are in /Library/Documentation - and runs
-- uninstall.sh as root; the settings, the person's own, it offers to remove as the person.

set theTitle to "Uninstall Reckless Sudoku"
set theList to "• Reckless Sudoku and this uninstaller in /Applications/Reckless Technology" & return & ¬
	"• its licence and notices in /Library/Documentation"
display dialog "This removes Reckless Sudoku from this Mac:" & return & return & theList & return & return & ¬
	"The other Reckless products are kept." with title theTitle ¬
	buttons {"Cancel", "Uninstall"} default button "Cancel" cancel button "Cancel" with icon caution

set settingsChoice to button returned of (display dialog ¬
	"Remove Reckless Sudoku's settings too - the pencil marks and the window's size you chose?" ¬
	with title theTitle buttons {"Keep settings", "Remove settings"} default button "Keep settings")

-- The program quit first, so it neither runs on from a deleted bundle nor writes its settings back afterwards.
if application id "com.recklesstechnology.sudoku" is running then
	tell application id "com.recklesstechnology.sudoku" to quit
	delay 1
end if

set theScript to quoted form of POSIX path of (path to resource "uninstall.sh")
try
	-- An empty environment: nothing a shell of the person's set reaches the script root runs.
	set removed to do shell script "/usr/bin/env -i /bin/bash " & theScript with administrator privileges
on error errorText number errorNumber
	if errorNumber is -128 then return -- the password asked for and cancelled: nothing was touched
	display dialog "Reckless Sudoku could not be removed: " & errorText with title theTitle buttons {"OK"} default button "OK" with icon stop
	return
end try

if settingsChoice is "Remove settings" then
	-- The program's own folder alone: the folder above is shared with the other products' settings.
	do shell script "cd \"$HOME/Library/Application Support/reckless-technology\" 2>/dev/null && /bin/rm -rf sudoku; /bin/rmdir \"$HOME/Library/Application Support/reckless-technology\" 2>/dev/null; true"
end if

display dialog "Reckless Sudoku is removed." with title theTitle buttons {"OK"} default button "OK"

Module:	HDAEnabler
Description: This module provides a substitute for the Hight Definition Audio DSDT Edits (HDEF and HDAU).
Dependencies: none


Usage:
- Copy the module HDAEnabler.dylib into /Extra/modules folder
- Set HDAEnabler=Yes (Default value is YES if the module is in modules folder)
- Set HDEFLayoutID="a hex value" for HDEF layout to use with a patched AppleHDA kext.
- Set HDAULayoutID="a hex value" for HDAU layout to use with a patched AppleHDA kext.

-----
- HDAEnabler=No (Disable the HDAEnabler module with module in modules folder).


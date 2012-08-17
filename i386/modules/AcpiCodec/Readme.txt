Module:	ACPICodec
Description: This module provides a remplacement for the internal acpi patcher
Dependencies: none

Help:

AcpiCodec module: (Warning: acpi signature not implemented)
------------------
  IMPORTANT NOTE: 1- with AcpiCodec all aml files must be placed in /Extra/Acpi/, alternate or overridden path is no longer supported
                  
                  2- the name of the aml file(s) is not important anymore but it must contain the extention .aml, for example let suppose that you have 3 aml files: dsdt.aml, ssdt-0.aml and ssdt-1.aml
                     you can rename your dsdt file into blablabr.aml, and rename your ssdt files into blablablaen.aml and blablablablada.aml, acpicodec will auto-magically detect those files as 2 ssdt and one dsdt
    
  RestartFix=Yes|No		Enable/Disable internal restart fix patch (Enabled by default, only available for intel platform).

  ResetType=0|1			0 : PCI reset (Default)
						1 : keyboard reset 

  EnableSpeedStep=Yes|No	Enable/Disable GenerateCStates & GeneratePStates (Disabled by default).
  
  GeneratePStates=Yes|No	Enable/Disable Generate P-states SSDT table (Disabled by default).
  GenerateCStates=Yes|No	Enable/Disable Generate C-states SSDT table (Disabled by default).
  EnableC4State=Yes|No		Enable C4 state in C-states SSDT table, GenerateCStates=Yes is needed (Disabled by default).
  
  StripAPICTable=Yes|No		Enable/Disable Generate a stripped MADT (APIC) table (Enabled by default).

  IntelFADTSpec=Yes|No		Enable/Disable Intel recommendations for the FADT table (Enabled by default). 
							Warning : When enabled, this setting disable the C2 and C3 C-states, but be aware that these are the Intel's recommendations for the newest CPU, 
							if you really need those c-states please disable IntelFADTSpec.

  P-States=<dict>		P-States fine tuning method, see usage below(GeneratePStates=Yes is needed).
  ACPIDropTables=<dict>		drop acpi table(s) method (can drop any unwanted tables), see usage below.
  C-States=<dict>		C-States fine tuning method, see usage below(GenerateCStates=Yes is needed).
  
  							
  UpdateACPI=Yes|No    Enable/Disable ACPI version update(Disabled by default). 			                

  MaxBusRatio=<n>	(was BusRatio) Set the current Bus Ratio to n, 
			        n must be a multiple of 10,
					(eg. if you want to set a bus ratio to 8.5, n will be
			        8.5*10=85), 
			        if n = 0, MaxBusRatio = Disable,
			        if set, Acpipatcher will drop every P-states with
			        a bus ratio higher than n.
						  
  MinBusRatio=<n>	Set the Minimum Bus Ratio to n,
			        n must be a multiple of 10,
			        (eg. if you want to set the bus ratio to 8.5, n will be
			        8.5*10=85), if set Acpipatcher will drop every 
			        P-states with a bus ratio lower than n, 
			        if n = 0, MinBusRatio = Disable.
		  
  P-States usage e.g: (by default all numbers must be expressed in base 16, 
			          except the pss statue key and base key itself)
		
		<key>P-States</key>
		<dict>
		<key>0</key> // the pss status (must be expressed in Base 10)
		<dict>
		<key>Bus Master Latency</key>
		<string>10</string>
		<key>Control</key>
		<string>18719</string>
		<key>CoreFreq</key>
		<string>3164</string>
		<key>Transition Latency</key>
		<string>10</string>
		</dict>
        <key>1</key> // the pss status (must be expressed in Base 10)
        .
        .
        .
		<dict/>
		<key>2</key> // the pss status (must be expressed in Base 10)
        .
        .
        .
		<dict/>
        <key>X</key> // the pss status (must be expressed in Base 10)
        .
        .
        .
		<dict/>
		<key>Base</key>
		<string>10</string>  // must always be expressed in Base 10 
		<key>Mode</key>
		<string>Default</string>
		</dict> 	
									 

  C-States usage e.g: (by default all numbers must be expressed in base 16, 
			          except the base key itself)

		<key>C-states</key>
		<dict>
		<key>C1</key>
		<dict>
		<key>Latency</key>
		<string>THE LATENCY FOR THIS STATE</string>
		<key>Power</key>
		<string>THE POWER FOR THIS STATE</string>
		</dict>
		.
		.							
		<key>C4</key>
		<dict>
		<key>Latency</key>
		<string>THE LATENCY FOR THIS STATE</string>
		<key>Power</key>
		<string>THE POWER FOR THIS STATE</string>
		</dict>
		</dict>
		
  ACPIDropTables usage e.g: 

		<key>ACPIDropTables</key>
		<dict>
		<key>SSDT</key>
		<string></string> // drop SSDT table(s)
		<key>TAMG</key>
		<string>Yes</string> //drop TAMG table
		<key>ECDT</key>
		<string>ANY_VALUE_EXCEPT_NO</string> //drop ECDT table
		<key>XXXX</key>
		<string></string> //drop XXXX table (if exist)
		<key>YYYY</key>
		<string>No</string> //do not drop YYYY table (if exist)
		</dict>

EE2D707564081AB603703E236BBA252A8F712D0B2BA5D7AB3D4DFDB59C97570912EBD6FEE4868CEE130E8473FECE30BD272128A255BE1DFDB9CEB0FAF0504B0102140314030100630004731141000000007D0000008B00000003000B00000000000000208080810000000062696E0199070002004145030800504B050600000000010001003C000000A90000000000

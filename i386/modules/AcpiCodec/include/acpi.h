#ifndef __LIBSAIO_ACPI_H
#define __LIBSAIO_ACPI_H

#include "acpi_tools.h"

#define ACPI_RANGE_START    (0x0E0000)
#define ACPI_RANGE_END      (0x0FFFFF)


#define Unspecified         0
#define Desktop             1
#define Mobile              2
#define Workstation         3
#define EnterpriseServer    4
#define SOHOServer          5
#define AppliancePC         6
#define PerformanceServer   7

#define MaxSupportedPMProfile     PerformanceServer // currently max profile supported 
#define PMProfileError            MaxSupportedPMProfile + 1

#define kAcpiMethod			"Acpi2Method"				// 2 (= method 2) for some machines that may hang on acpi 2 (aka acpi 64 bit) detection (replace the old kUnsafeACPI "UnsafeACPI") 	


#endif /* !__LIBSAIO_ACPI_H */

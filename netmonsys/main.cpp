
#include "stdafx.h"
#include <ntifs.h>
#include <kinit.h>
#include <kframe/frame.h>
#include <kframe/simple_drive.h>

#include "NetMonDevice.h"

#include <jansson/jansson.hpp>

using namespace msddk;
DRIVER_ENTRY(SimpleDrive<CNetMonDevice>);
#include "rclcomm.h"
extern "C" {
VirtualChannel *GetChannelInstance() { return new rclcomm(); }
}
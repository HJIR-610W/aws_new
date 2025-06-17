

#include "aws_monitor.h"

void make_aws_monitor_frame(aws_monitor_t *p_monitor)
{
  p_monitor->cdma_system = *get_cdma_system();
  p_monitor->system = *get_system();
  p_monitor->eth_system[ETH_CLIENT_0] = *get_tcp_system(ETH_CLIENT_0);
  p_monitor->eth_system[ETH_CLIENT_1] = *get_tcp_system(ETH_CLIENT_1);
}
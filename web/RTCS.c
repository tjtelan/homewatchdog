/**HEADER********************************************************************
*
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 2004-2008 Embedded Access Inc.;
* All Rights Reserved
*
***************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* Fileame: rtcs.c
* Modified by : T.J. Telan
* Date : 18 May 2011
*
* Comments: Change mount points for web server. Be careful. Can be confusing.

*END************************************************************************/

#include "httpd_types.h"
#include "ifconfig.h"
#include  <ipcfg.h>


/* Define file structures to store HTML pages */

/* Use:
 * { "name of directory", "mount point" }
 * The default point for the SDCard on the demo board is "a:"
 * "" point refers to root of SDCard and webroot
 * ex. 
 * "" = http://192.168.0.1/
 *
 * or
 * "directory" = http://192.168.0.1/
 * Which searches in "directory" on the SDCard for files to serve.
 *
 * Can also use trivial file system, which is "tfs:"
 * Consult MQX docs for how to use the tools for tfs 
 *
 * You only need one root per mount point.
 * Refer to the Freescale demo labs for clarification on use
 */
const HTTPD_ROOT_DIR_STRUCT root_dir[] = {
    { "", "a:" },
    { 0, 0 }
};

/* Initialize the network stack and start webserver
 *
 * IP, subnet and gateway are set in ifconfig.h 
 *
 * Leave this alone, unless you know what you are doing*/
void initialize_networking(void)
{
    int_32                  error;
    IPCFG_IP_ADDRESS_DATA   ip_data;

   /* runtime RTCS configuration */
   _RTCSPCB_init = 4;
   _RTCSPCB_grow = 2;
   _RTCSPCB_max = 20;
   _RTCS_msgpool_init = 4;
   _RTCS_msgpool_grow = 2;
   _RTCS_msgpool_max  = 20;
   _RTCS_socket_part_init = 4;
   _RTCS_socket_part_grow = 2;
   _RTCS_socket_part_max  = 20;

    error = RTCS_create();

    IPCFG_default_enet_device = DEMOCFG_DEFAULT_DEVICE;
    IPCFG_default_ip_address = ENET_IPADDR;
    IPCFG_default_ip_mask = ENET_IPMASK;
    IPCFG_default_ip_gateway = ENET_IPGATEWAY;
    LWDNS_server_ipaddr = ENET_IPGATEWAY;

    ip_data.ip = IPCFG_default_ip_address;
    ip_data.mask = IPCFG_default_ip_mask;
    ip_data.gateway = IPCFG_default_ip_gateway;

    ENET_get_mac_address (IPCFG_default_enet_device, IPCFG_default_ip_address, IPCFG_default_enet_address);
    error = ipcfg_init_device (IPCFG_default_enet_device, IPCFG_default_enet_address);

#if DEMOCFG_ENABLE_DHCP
    error = ipcfg_bind_dhcp_wait(IPCFG_default_enet_device, 1, &ip_data);
    if (error != IPCFG_ERROR_OK)
    {
      printf("DHCP Error %08x!\n", error);
    }
#else
    error = ipcfg_bind_staticip (IPCFG_default_enet_device, &ip_data);
    if (error != IPCFG_ERROR_OK)
    {
      printf("Static IP Error %08x!\n", error);
    }
#endif

    ipcfg_get_ip(IPCFG_default_enet_device, &ip_data);
    printf("IP Address      : %d.%d.%d.%d\n\n",IPBYTES(ip_data.ip));

#if DEMOCFG_ENABLE_WEBSERVER

        HTTPD_STRUCT *server;
        extern const HTTPD_CGI_LINK_STRUCT cgi_lnk_tbl[];
        extern const HTTPD_FN_LINK_STRUCT fn_lnk_tbl[];

        /* Change pathname to point root to different file
        ex. IP address is 192.168.0.100 & file is index.html
        Browsing to http://192.168.0.100 will automatically go to index.html*/
        server = httpd_server_init((HTTPD_ROOT_DIR_STRUCT*)root_dir, "about.html");
        HTTPD_SET_PARAM_CGI_TBL(server, (HTTPD_CGI_LINK_STRUCT*)cgi_lnk_tbl);
        HTTPD_SET_PARAM_FN_TBL(server, (HTTPD_FN_LINK_STRUCT*)fn_lnk_tbl);

        httpd_server_run(server);

#endif

}

/* EOF */

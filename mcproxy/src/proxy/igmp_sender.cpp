/*
 * This file is part of mcproxy.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * written by Sebastian Woelke, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * Website: http://mcproxy.realmv6.org/
 */


#include "include/hamcast_logging.h"
#include "include/proxy/igmp_sender.hpp"

#include <netinet/igmp.h>
#include <netinet/ip.h>
#include <net/if.h>

#include <memory>

igmp_sender::igmp_sender(): sender(IGMPv3)
{
    HC_LOG_TRACE("");

}

bool igmp_sender::send_report(unsigned int if_index, mc_filter filter_mode, const addr_storage& gaddr, const source_list<source>& slist) const
{
    HC_LOG_TRACE("");


}

bool igmp_sender::send_general_query(unsigned int if_index, const timers_values& tv, group_mem_protocol gmp) const
{
    HC_LOG_TRACE("");


}

bool igmp_sender::send_mc_addr_specific_query(unsigned int if_index, const timers_values& tv, const addr_storage& gaddr, bool s_flag, group_mem_protocol gmp) const
{
    HC_LOG_TRACE("");



}

bool igmp_sender::send_mc_addr_and_src_specific_query(unsigned int if_index, const timers_values& tv, const addr_storage& gaddr, source_list<source>& slist, group_mem_protocol gmp) const
{
    HC_LOG_TRACE("");



}

//bool igmp_sender::send_general_query(int if_index) const
//{
//HC_LOG_TRACE("");

//int size = get_msg_min_size();
//if (size < 0) {
//return false;
//}

//std::unique_ptr<unsigned char[]> buf { new unsigned char[size] };
////unsigned char buf[size];

//if (!m_sock.choose_if(if_index)) {
//return false;
//}
//if (!create_mc_query(GENERAL_QUERY, buf.get())) {
//return false;
//}

//return m_sock.send_packet(addr_storage(IPV4_ALL_HOST_ADDR), buf.get(), size);
//}

//bool igmp_sender::send_group_specific_query(int if_index, const addr_storage& g_addr) const
//{
//HC_LOG_TRACE("");

//int size = get_msg_min_size();
//if (size < 0) {
//return false;
//}

//std::unique_ptr<unsigned char[]> buf { new unsigned char[size] };
////unsigned char buf[size];

//if (!m_sock.choose_if(if_index)) {
//return false;
//}
//if (!create_mc_query(GROUP_SPECIFIC_QUERY, buf.get(), &g_addr)) {
//return false;
//}

//return m_sock.send_packet(addr_storage(g_addr), buf.get(), size);
//}

//bool igmp_sender::send_report(int if_index, const addr_storage& g_addr) const
//{
//HC_LOG_TRACE("");

//return m_sock.join_group(g_addr, if_index);
//}

//bool igmp_sender::send_leave(int if_index, const addr_storage& g_addr) const
//{
//HC_LOG_TRACE("");

//return m_sock.leave_group(g_addr, if_index);
//}

//int igmp_sender::get_msg_min_size() const
//{
//HC_LOG_TRACE("");

////if (m_version == 2) {
////return sizeof(struct igmp);
////} else {
////HC_LOG_ERROR("IPv4 version: " << m_version << " not supported");
////return -1;
////}
//return -1;
//}


//bool igmp_sender::create_mc_query(msg_type type, unsigned char* buf, const addr_storage* g_addr) const
//{
//HC_LOG_TRACE("");

////if (m_version == 2) {
////struct igmp* igmp_Hdr = (struct igmp*)(buf);

////igmp_Hdr->igmp_type = IGMP_MEMBERSHIP_QUERY;
////igmp_Hdr->igmp_cksum = 0;

////if (type == GENERAL_QUERY) {
////igmp_Hdr->igmp_code = MC_TV_QUERY_RESPONSE_INTERVAL * MC_TV_MAX_RESPONSE_TIME_UNIT;
////igmp_Hdr->igmp_group = addr_storage(m_addr_family).get_in_addr(); //0.0.0.0
////} else if (type == GROUP_SPECIFIC_QUERY) {
////if (!g_addr) {
////HC_LOG_ERROR("g_addr is NULL");
////return false;
////}

////igmp_Hdr->igmp_code = MC_TV_LAST_MEMBER_QUERY_INTEVAL * MC_TV_MAX_RESPONSE_TIME_UNIT;
////igmp_Hdr->igmp_group = g_addr->get_in_addr();
////} else {
////HC_LOG_ERROR("wrong type: " << type);
////return false;
////}

////igmp_Hdr->igmp_cksum = m_sock.calc_checksum((unsigned char*)igmp_Hdr, sizeof(struct igmp));

////return true;
////} else {
////HC_LOG_ERROR("wrong verson: " << m_version);
////return false;
////}
//return true;
//}


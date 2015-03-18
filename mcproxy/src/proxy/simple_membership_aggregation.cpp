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
#include "include/proxy/simple_membership_aggregation.hpp"

//is this necessary?????????????
#include "include/proxy/proxy_instance.hpp"
#include "include/proxy/simple_routing_data.hpp"

#include <algorithm>
#include <utility>

mem_source_state::mem_source_state()
{
    HC_LOG_TRACE("");
}

mem_source_state::mem_source_state(mc_filter filter, const source_list<source>& slist)
    : m_mc_filter(filter)
    , m_source_list(slist)
{
    HC_LOG_TRACE("");
}

mem_source_state::mem_source_state(std::pair<mc_filter, source_list<source>> sstate)
    : mem_source_state(sstate.first, sstate.second)
{
    HC_LOG_TRACE("");
}

std::pair<mc_filter, const source_list<source>&> mem_source_state::get_mc_source_state() const
{
    HC_LOG_TRACE("");
    return std::pair<mc_filter, const source_list<source>&>(m_mc_filter, m_source_list);
}

bool mem_source_state::operator==(const mem_source_state& mss) const
{
    HC_LOG_TRACE("");
    return m_source_list == mss.m_source_list && m_mc_filter == mss.m_mc_filter;
}

bool mem_source_state::operator!=(const mem_source_state& mss) const
{
    HC_LOG_TRACE("");
    return !(*this == mss);
}

std::string mem_source_state::to_string() const
{
    HC_LOG_TRACE("");
    std::ostringstream s;
    s << get_mc_filter_name(m_mc_filter) << "{" << m_source_list << "}";
    return s.str();
}
//-----------------------------------------------------------------
filter_source_state::filter_source_state()
{
    HC_LOG_TRACE("");
}

filter_source_state::filter_source_state(rb_filter_type rb_filter, const source_list<source>& slist)
    : m_rb_filter(rb_filter)
    , m_source_list(slist)
{
    HC_LOG_TRACE("");
}

bool filter_source_state::operator==(const filter_source_state& mss) const
{
    HC_LOG_TRACE("");
    return m_source_list == mss.m_source_list && m_rb_filter == mss.m_rb_filter;
}

bool filter_source_state::operator!=(const filter_source_state& mss) const
{
    HC_LOG_TRACE("");
    return !(*this == mss);
}

std::string filter_source_state::to_string() const
{
    HC_LOG_TRACE("");
    std::ostringstream s;
    s << get_rb_filter_type_name(m_rb_filter) << "{" << m_source_list << "}";
    return s.str();
}

//-----------------------------------------------------------------

simple_membership_aggregation::simple_membership_aggregation(group_mem_protocol group_mem_protocol)
    : m_group_mem_protocol(group_mem_protocol)
    , m_routing_data(nullptr)
{
    HC_LOG_TRACE("");

}

simple_membership_aggregation::simple_membership_aggregation(rb_rule_matching_type upstream_in_rule_matching_type, const addr_storage& gaddr, const std::shared_ptr<const simple_routing_data>& routing_data, group_mem_protocol group_mem_protocol, const std::shared_ptr<const interface_infos>& interface_infos)
    : m_group_mem_protocol(group_mem_protocol)
    , m_ii(interface_infos)
    , m_routing_data(routing_data)

{
    HC_LOG_TRACE("");

    if (upstream_in_rule_matching_type == RMT_FIRST) {
        process_upstream_in_first(gaddr);
    } else if (upstream_in_rule_matching_type == RMT_MUTEX) {
        process_upstream_in_mutex(gaddr);
    } else {
        HC_LOG_ERROR("unkown upstream input rule matching type");
    }
}

void simple_membership_aggregation::set_to_block_all(mem_source_state& mc_groups) const
{
    mc_groups.m_source_list.clear();
    mc_groups.m_mc_filter = INCLUDE_MODE;
}

mem_source_state& simple_membership_aggregation::merge_group_memberships(mem_source_state& merge_to_mc_group, const mem_source_state& merge_from_mc_group) const
{
    HC_LOG_TRACE("");

    if (merge_to_mc_group.m_mc_filter == INCLUDE_MODE) {
        if (merge_from_mc_group.m_mc_filter == INCLUDE_MODE) {
            merge_to_mc_group.m_source_list += merge_from_mc_group.m_source_list;
        } else if (merge_from_mc_group.m_mc_filter == EXCLUDE_MODE) {
            merge_to_mc_group.m_mc_filter = EXCLUDE_MODE;
            merge_to_mc_group.m_source_list = merge_from_mc_group.m_source_list - merge_to_mc_group.m_source_list;
        } else {
            HC_LOG_ERROR("unknown mc filter mode in parameter merge_from_mc_group");
            set_to_block_all(merge_to_mc_group);
        }
    } else if (merge_to_mc_group.m_mc_filter == EXCLUDE_MODE) {
        if (merge_from_mc_group.m_mc_filter == INCLUDE_MODE) {
            merge_to_mc_group.m_source_list -= merge_from_mc_group.m_source_list;
        } else if (merge_from_mc_group.m_mc_filter == EXCLUDE_MODE) {
            merge_to_mc_group.m_source_list *= merge_from_mc_group.m_source_list;
        } else {
            HC_LOG_ERROR("unknown mc filter mode in parameter merge_from_mc_group");
            set_to_block_all(merge_to_mc_group);
        }
    } else {
        HC_LOG_ERROR("unknown mc filter mode in parameter merge_to_mc_group");
        set_to_block_all(merge_to_mc_group);
    }

    return merge_to_mc_group;
}

filter_source_state& simple_membership_aggregation::convert_wildcard_filter(filter_source_state& rb_filter) const
{
    HC_LOG_TRACE("");

    //if contains wildcard address
    if (rb_filter.m_source_list.find(addr_storage(get_addr_family(m_group_mem_protocol))) != std::end(rb_filter.m_source_list)) {
        if (rb_filter.m_rb_filter == FT_WHITELIST) { //WL{*} ==> BL{}
            rb_filter.m_rb_filter = FT_BLACKLIST;
            rb_filter.m_source_list.clear();
        } else if (rb_filter.m_rb_filter == FT_BLACKLIST) { //BL{*} ==> WL{}
            rb_filter.m_rb_filter = FT_WHITELIST;
            rb_filter.m_source_list.clear();
        } else {
            HC_LOG_ERROR("unknown rb filter mode in parameter merge_from_rb_filter");
        }
    }

    return rb_filter;
}


mem_source_state& simple_membership_aggregation::merge_memberships_filter(mem_source_state& merge_to_mc_group, const filter_source_state& merge_from_rb_filter) const
{
    HC_LOG_TRACE("");
    filter_source_state from = merge_from_rb_filter;
    //not very efficient ?????
    convert_wildcard_filter(from);

    if (merge_to_mc_group.m_mc_filter == INCLUDE_MODE) {
        if (from.m_rb_filter == FT_WHITELIST) {
            merge_to_mc_group.m_source_list *= from.m_source_list;
        } else if (from.m_rb_filter == FT_BLACKLIST) {
            merge_to_mc_group.m_source_list -= from.m_source_list;
        } else {
            HC_LOG_ERROR("unknown rb filter mode in parameter merge_from_rb_filter");
            set_to_block_all(merge_to_mc_group);
        }
    } else if (merge_to_mc_group.m_mc_filter == EXCLUDE_MODE) {
        if (from.m_rb_filter == FT_WHITELIST) {
            merge_to_mc_group.m_mc_filter = INCLUDE_MODE;
            merge_to_mc_group.m_source_list = from.m_source_list - merge_to_mc_group.m_source_list;
        } else if (from.m_rb_filter == FT_BLACKLIST) {
            merge_to_mc_group.m_source_list += from.m_source_list;
        } else {
            HC_LOG_ERROR("unknown rb filter mode in parameter merge_from_rb_filter");
            set_to_block_all(merge_to_mc_group);
        }
    } else {
        HC_LOG_ERROR("unknown mc filter mode in parameter merge_to_mc_group");
        set_to_block_all(merge_to_mc_group);
    }

    return merge_to_mc_group;
}

mem_source_state& simple_membership_aggregation::merge_memberships_filter_reminder(mem_source_state& merge_to_mc_group, const mem_source_state& result, const filter_source_state& merge_from_rb_filter) const
{
    HC_LOG_TRACE("");
    filter_source_state from = merge_from_rb_filter;
    //not very efficient ?????
    convert_wildcard_filter(from);

    if (merge_to_mc_group.m_mc_filter == INCLUDE_MODE) {
        if (from.m_rb_filter == FT_WHITELIST || from.m_rb_filter == FT_BLACKLIST) {
            merge_to_mc_group.m_source_list -= result.m_source_list;
        } else {
            HC_LOG_ERROR("unknown rb filter mode in parameter merge_from_rb_filter");
        }
    } else if (merge_to_mc_group.m_mc_filter == EXCLUDE_MODE) {
        if (from.m_rb_filter == FT_WHITELIST) {
            merge_to_mc_group.m_source_list += result.m_source_list;
        } else if (from.m_rb_filter == FT_BLACKLIST) {
            merge_to_mc_group.m_source_list = result.m_source_list - merge_to_mc_group.m_source_list;
            merge_to_mc_group.m_mc_filter = INCLUDE_MODE;
        } else {
            HC_LOG_ERROR("unknown rb filter mode in parameter merge_from_rb_filter");
        }
    } else {
        HC_LOG_ERROR("unknown mc filter mode in parameter merge_to_mc_group");
    }

    return merge_to_mc_group;
}

mem_source_state& simple_membership_aggregation::disjoin_group_memberships(mem_source_state& merge_to_mc_group, const mem_source_state& merge_from_mc_group) const
{
    HC_LOG_TRACE("");

    if (merge_to_mc_group.m_mc_filter == INCLUDE_MODE) {
        if (merge_from_mc_group.m_mc_filter == INCLUDE_MODE) {
            merge_to_mc_group.m_source_list -= merge_from_mc_group.m_source_list;
        } else if (merge_from_mc_group.m_mc_filter == EXCLUDE_MODE) {
            merge_to_mc_group.m_source_list *= merge_from_mc_group.m_source_list;
        } else {
            HC_LOG_ERROR("unknown mc filter mode in parameter merge_from_mc_group");
            set_to_block_all(merge_to_mc_group);
        }
    } else if (merge_to_mc_group.m_mc_filter == EXCLUDE_MODE) {
        if (merge_from_mc_group.m_mc_filter == INCLUDE_MODE) {
            merge_to_mc_group.m_source_list += merge_from_mc_group.m_source_list;
        } else if (merge_from_mc_group.m_mc_filter == EXCLUDE_MODE) {
            merge_to_mc_group.m_source_list = merge_from_mc_group.m_source_list - merge_to_mc_group.m_source_list;
            merge_to_mc_group.m_mc_filter = INCLUDE_MODE;
        } else {
            HC_LOG_ERROR("unknown mc filter mode in parameter merge_from_mc_group");
            set_to_block_all(merge_to_mc_group);
        }
    } else {
        HC_LOG_ERROR("unknown mc filter mode in parameter merge_to_mc_group");
        set_to_block_all(merge_to_mc_group);
    }

    return merge_to_mc_group;
}


source_list<source> simple_membership_aggregation::get_source_list(const std::set<addr_storage>& addr_set)
{
    HC_LOG_TRACE("");
    source_list<source> result;
    //a reverse iteration is maybe faster????????????
    std::for_each(addr_set.begin(), addr_set.end(), [&result](const addr_storage & e) {
        result.insert(e);
    });
    return result;
}

filter_source_state simple_membership_aggregation::get_source_filter(rb_interface_direction if_direction, const std::string& input_if_name, const std::shared_ptr<interface>& iface, const addr_storage& gaddr, bool explicit_if_name)
{
    HC_LOG_TRACE("");
    rb_filter_type tmp_ft = iface->get_filter_type(if_direction);
    source_list<source> tmp_srcl = get_source_list(iface->get_saddr_set(if_direction, input_if_name, gaddr, explicit_if_name));
    return filter_source_state(tmp_ft, tmp_srcl);
}

mem_source_state simple_membership_aggregation::get_mem_source_state(const std::unique_ptr<querier>& querier, const addr_storage& gaddr)
{
    HC_LOG_TRACE("");
    return mem_source_state(querier->get_group_membership_infos(gaddr));
}

std::map<unsigned int, mem_source_state> simple_membership_aggregation::get_merged_mem(const addr_storage& gaddr)
{
    HC_LOG_TRACE("");

    //filters group memberships of the queries with the interface filter
    auto get_filtered_mem = [&gaddr, this](const std::pair<const unsigned int, downstream_infos>& dstream, const std::string & input_if_name, bool explicit_if_name) {
        auto filter = get_source_filter(ID_OUT, input_if_name, dstream.second.m_interface, gaddr, explicit_if_name);
        auto gmem_src = get_mem_source_state(dstream.second.m_querier, gaddr);
        merge_memberships_filter(gmem_src, filter); //(p1)
        std::cout << "get_filtered_mem: " <<  dstream.second.m_interface->get_if_name() << " " << gmem_src.to_string() << std::endl;
        return gmem_src;
    };

    //upstream interface index, membership infos
    std::map<unsigned int, mem_source_state> downstreams_mem_merge; //(p3)
    mem_source_state downstreams_merge_empty_if; //(p2

    for (auto & dstream : m_ii->m_downstreams) {
        merge_group_memberships(downstreams_merge_empty_if, get_filtered_mem(dstream, "", false)); //(p2)
    }

    std::cout << "downstreams_merge_empty_if: "  << downstreams_merge_empty_if.to_string() << std::endl;

    for (auto & ustream : m_ii->m_upstreams) {
        mem_source_state if_specific_merge;
        std::string uif_name = ustream.m_interface->get_if_name();
        for (auto & dstream : m_ii->m_downstreams) {
            //get_filtered_mem --> get_mem_source_state is called twice ??????
            merge_group_memberships(if_specific_merge, get_filtered_mem(dstream, uif_name, true)); //(p2)
        }
        merge_group_memberships(if_specific_merge, downstreams_merge_empty_if);
        downstreams_mem_merge.insert({ustream.m_if_index, std::move(if_specific_merge)});
    }

    return downstreams_mem_merge;
}


void simple_membership_aggregation::process_upstream_in_first(const addr_storage& gaddr)
{
    HC_LOG_TRACE("");
    std::cout << "in process_upstream_in_first" << std::endl;
    std::map<unsigned int, mem_source_state> upstream_mem_merge = get_merged_mem(gaddr); //(p3) and result
    for(auto & e: upstream_mem_merge){
        std::cout << "uif: " << e.first << " " << e.second.to_string() << std::endl;
    }

    mem_source_state disjoiner;
    for (auto & ustream : m_ii->m_upstreams) {
        auto d_mem_merge_it = upstream_mem_merge.find(ustream.m_if_index);
        if (d_mem_merge_it == upstream_mem_merge.end()) {
            HC_LOG_ERROR("upstream interface index (" << ustream.m_if_index << ") not found");
            upstream_mem_merge.erase(d_mem_merge_it);
            continue;
        }

        mem_source_state& d_mem_merge = d_mem_merge_it->second; //(p3)

        auto filter = get_source_filter(ID_IN, ustream.m_interface->get_if_name(),  ustream.m_interface, gaddr, false);
        merge_memberships_filter(d_mem_merge, filter);
        disjoin_group_memberships(d_mem_merge, disjoiner);
        merge_group_memberships(disjoiner, d_mem_merge);
    }

    m_data = std::move(upstream_mem_merge);
}


void simple_membership_aggregation::process_upstream_in_mutex(const addr_storage& gaddr)
{
    std::map<unsigned int, mem_source_state> upstream_mem_merge = get_merged_mem(gaddr); //(p3) and result
    std::map<unsigned int, mem_source_state> ustream_recvd_source_map;
    mem_source_state dstream_recvd_sources;

    const std::map<addr_storage, unsigned int>& tmp_if_map = m_routing_data->get_interface_map(gaddr);

    //1.) convert m_routing_data->get_interface_map(gaddr) to up- and downstream_recvd_source;
    for (auto & ustream : m_ii->m_upstreams) {
        ustream_recvd_source_map.insert({ustream.m_if_index, mem_source_state(INCLUDE_MODE, source_list<source> {})});
    }

    for (auto & e : tmp_if_map) {
        auto ursm_it = ustream_recvd_source_map.find(e.second);
        if (ursm_it == ustream_recvd_source_map.end()) {
            dstream_recvd_sources.m_source_list.insert(e.first);
        } else {
            for (auto uit = ustream_recvd_source_map.begin(); uit != ustream_recvd_source_map.end(); ++uit) {
                if (uit != ursm_it) {
                    uit->second.m_source_list.insert(e.first);
                }
            }
        }
    }

    //2.) disjoin and merge with interface filters
    for (auto & ustream : m_ii->m_upstreams) {
        auto d_mem_merge_it = upstream_mem_merge.find(ustream.m_if_index);
        if (d_mem_merge_it == upstream_mem_merge.end()) {
            HC_LOG_ERROR("upstream interface index (" << ustream.m_if_index << ") in upstream_mem_merge not found");
            upstream_mem_merge.erase(d_mem_merge_it);
            continue;
        }

        auto ursm_it = ustream_recvd_source_map.find(ustream.m_if_index);
        if (ursm_it == ustream_recvd_source_map.end()) {
            HC_LOG_ERROR("upstream interface index (" << ustream.m_if_index << ") in ustream_recvd_source_map not found");
            continue;
        }

        auto filter = get_source_filter(ID_IN, ustream.m_interface->get_if_name(), ustream.m_interface, gaddr, false);
        mem_source_state& d_mem_merge = d_mem_merge_it->second;

        merge_group_memberships(ursm_it->second, dstream_recvd_sources);
        disjoin_group_memberships(d_mem_merge, ursm_it->second);
        merge_memberships_filter(d_mem_merge, filter);
    }

    m_data = std::move(upstream_mem_merge);
}

std::pair<mc_filter, const source_list<source>&> simple_membership_aggregation::get_group_memberships(unsigned int upstream_if_index) const
{
    HC_LOG_TRACE("");
    auto tmp_it = m_data.find(upstream_if_index);
    if (tmp_it != m_data.end()) {
        return {tmp_it->second.m_mc_filter, tmp_it->second.m_source_list};
    } else {
        HC_LOG_ERROR("upstream if index " << upstream_if_index << " not found");
        return {INCLUDE_MODE, m_fallback_list};
    }
}

simple_membership_aggregation::~simple_membership_aggregation()
{
    HC_LOG_TRACE("");
}

std::string simple_membership_aggregation::to_string() const
{
    std::ostringstream s;
    for (auto & e : m_ii->m_upstreams) {
        s <<  e.m_interface->get_if_name() << "(" << e.m_if_index << ")" << ":";
        auto smem_it = m_data.find(e.m_if_index);
        if (smem_it != m_data.end()) {
            s << std::endl << indention(smem_it->second.to_string());
        }
    }
    return s.str();
}


//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <map>
#include "MACAddressTable.h"

Define_Module(MACAddressTable);

MACAddressTable::MACAddressTable()
{
    addressTable = new AddressTable();

    // Set addressTable for VLAN ID 0
    vlanAddressTable[0] = addressTable;
}

void MACAddressTable::initialize()
{
    agingTime = par("agingTime");
}

void MACAddressTable::handleMessage(cMessage *)
{
    opp_error("This module doesn't process messages");
}

/*
 * getTableForVid
 * Returns a MAC Address Table for a specified VLAN ID
 * or NULL pointer if it is not found
 */

MACAddressTable::AddressTable * MACAddressTable::getTableForVid(unsigned int vid)
{
    if (vid == 0)
        return addressTable;

    MACAddressTable::VlanAddressTable::iterator iter;
    iter = vlanAddressTable.find(vid);
    if (iter != vlanAddressTable.end())
        return iter->second;
    return NULL;
}

/*
 * getPortForAddress
 * For a known arriving port, V-TAG and destination MAC. It generates a vector with the ports where relay component
 * should deliver the message.
 * returns false if not found
 */

int MACAddressTable::getPortForAddress(MACAddress& address, unsigned int vid)
{
    Enter_Method("MACAddressTable::getPortForAddress()");

    AddressTable * table = getTableForVid(vid);
    // VLAN ID vid does not exist
    if (table == NULL)
        return -1;

    AddressTable::iterator iter = table->find(address);

    if (iter == table->end())
    {
        // not found
        return -1;
    }
    if (iter->second.insertionTime + agingTime <= simTime())
    {
        // don't use (and throw out) aged entries
        EV<< "Ignoring and deleting aged entry: "<< iter->first << " --> port" << iter->second.portno << "\n";
        table->erase(iter);
        return -1;
    }
    return iter->second.portno;
}

/*
 * updateTableWithAddress
 * Register a new MAC address at addressTable.
 * True if refreshed. False if it is new.
 */

bool MACAddressTable::updateTableWithAddress(int portno, MACAddress& address, unsigned int vid)
{
    Enter_Method("MACAddressTable::updateTableWithAddress()");
    if (address.isBroadcast())
        return false;

    AddressTable::iterator iter;
    AddressTable * table = getTableForVid(vid);
    if (table == NULL)
    {
        // MAC Address Table does not exist for VLAN ID vid, so we create it
        table = new AddressTable();
        vlanAddressTable[vid] = table;
        iter = table->end();
    }
    else
        iter = table->find(address);

    if (iter == table->end())
    {
        // Add entry to table
        EV<< "Adding entry to Address Table: "<< address << " --> port" << portno << "\n";
        (*table)[address] = AddressEntry(vid,portno,simTime());
        return false;
    }
    else
    {
        // Update existing entry
        EV << "Updating entry in Address Table: "<< address << " --> port" << portno << "\n";
        AddressEntry& entry = iter->second;
        entry.insertionTime = simTime();
        entry.portno = portno;
    }
    return true;
}

/*
 * Clears portno MAC cache.
 */

void MACAddressTable::flush(int portno)
{
    Enter_Method("MACAddressTable::flush():  Clearing gate %d cache", portno);
    for (VlanAddressTable::iterator i = vlanAddressTable.begin(); i != vlanAddressTable.end(); i++)
    {
        AddressTable * table = i->second;
        for (AddressTable::iterator j = table->begin(); j != table->end();)
        {
            AddressTable::iterator cur = j++;
            if (j->second.portno == portno)
                table->erase(cur);
        }

    }
}
/*
 * Prints verbose information
 */

void MACAddressTable::printState()
{
    EV<< endl << "MAC Address Table" << endl;
    EV << "VLAN ID    MAC    Port    Inserted" << endl;
    for (VlanAddressTable::iterator i = vlanAddressTable.begin(); i != vlanAddressTable.end(); i++)
    {
        AddressTable * table = i->second;
        for (AddressTable::iterator j = table->begin(); j != table->end(); j++)
        EV << j->second.vid << "   " << j->first << "   " << j->second.portno << "   " << j->second.insertionTime << endl;
    }

}

void MACAddressTable::copyTable(int portA, int portB)
{
    for (VlanAddressTable::iterator i = vlanAddressTable.begin(); i != vlanAddressTable.end(); i++)
    {
        AddressTable * table = i->second;
        for (AddressTable::iterator j = table->begin(); j != table->end(); j++)
            if (j->second.portno == portA)
                j->second.portno = portB;
    }
}

void MACAddressTable::removeAgedEntriesFromVlan(unsigned int vid)
{
    AddressTable * table = getTableForVid(vid);
    if (table == NULL)
        return;
    for (AddressTable::iterator iter = table->begin(); iter != table->end();)
    {
        AddressTable::iterator cur = iter++; // iter will get invalidated after erase()
        AddressEntry& entry = cur->second;
        if (entry.insertionTime + agingTime <= simTime())
        {
            EV<< "Removing aged entry from Address Table: " <<
            cur->first << " --> port" << cur->second.portno << "\n";
            table->erase(cur);
        }
    }
}

void MACAddressTable::removeAgedEntriesFromAllVlans()
{
    for (VlanAddressTable::iterator i = vlanAddressTable.begin(); i != vlanAddressTable.end(); i++)
    {
        AddressTable * table = i->second;
        for (AddressTable::iterator j = table->begin(); j != table->end();)
        {
            AddressTable::iterator cur = j++; // iter will get invalidated after erase()
            AddressEntry& entry = cur->second;
            if (entry.insertionTime + agingTime <= simTime())
            {
                EV<< "Removing aged entry from Address Table: " <<
                cur->first << " --> port" << cur->second.portno << "\n";
                table->erase(cur);
            }
        }
    }
}

MACAddressTable::~MACAddressTable()
{
    for (VlanAddressTable::iterator iter = vlanAddressTable.begin(); iter != vlanAddressTable.end(); iter++)
        delete iter->second;
}
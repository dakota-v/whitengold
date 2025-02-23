//---------------------------------------------------------------------

#include "FirstSharedNetwork.h"
#include "Address.h"
#include <cassert>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>

//---------------------------------------------------------------------

Address::Address() :
addr4(new struct sockaddr_in),
hostAddress("0.0.0.0")
{
	memset(addr4, 0, sizeof(struct sockaddr_in));
	addr4->sin_family = AF_INET;
}

//---------------------------------------------------------------------

Address::Address(const std::string & newHostAddress, unsigned short newHostPort) :
addr4(new struct sockaddr_in),
hostAddress(newHostAddress)
{
	struct hostent * h;
	unsigned long    u;

	memset(addr4, 0, sizeof(struct sockaddr_in));
	addr4->sin_port = htons(newHostPort);
	addr4->sin_family = AF_INET;
	// was an address supplied?
	if(hostAddress.size() > 0)
	{
		// Is the first byte a number? (IP names begin with an alpha)
		if(!isdigit(hostAddress[0]))
		{
			// The first byte is a letter, resolve it
			if( (h = gethostbyname(hostAddress.c_str())) != 0)
			{
				memcpy(&addr4->sin_addr, h->h_addr_list[0], sizeof(addr4->sin_addr));
			}
			else
			{
				// boom! grab the entry from the h_addr member instead!
				if( (h = gethostbyname(hostAddress.c_str())) != 0)
				{
					memcpy(&addr4->sin_addr, h->h_addr, sizeof(addr4->sin_addr));
				}
				else
				{
					// no resolution, INADDR_ANY
					memset(&addr4->sin_addr, 0, sizeof(addr4->sin_addr));

					// in debug, fail, something is wrong.
					// potential problems - an insanely bogus address was
					// passed or the host system is misconfigured and cannot
					// resolve the name
					assert(false);
				}
			}
			char addrbuf[17] = {"\0"};
			unsigned char * a = (unsigned char *)&addr4->sin_addr;
			snprintf(addrbuf, sizeof(addrbuf), "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
			hostAddress = addrbuf;
		}
		else
		{
			// A dotted decimal ip number string was supplied. Convert for sin_addr
			u = inet_addr(hostAddress.c_str());
			memcpy(&addr4->sin_addr, &u, sizeof(addr4->sin_addr));
		}
	}
	else
	{
		// nothing was supplied, assign INADDR_ANY
		addr4->sin_addr.s_addr = INADDR_ANY;
	}
}

//---------------------------------------------------------------------

Address::Address(const Address & source) :
addr4(new struct sockaddr_in),
hostAddress(source.hostAddress)
{
	*addr4 = *source.addr4;
}

//---------------------------------------------------------------------

Address::Address(const struct sockaddr_in & ipv4addr) :
addr4(new struct sockaddr_in),
hostAddress("")
{
	convertFromSockAddr(ipv4addr);
}

//---------------------------------------------------------------------

Address::~Address()
{
	delete addr4;
}

//---------------------------------------------------------------------

Address & Address::operator = (const Address & rhs)
{
	if(this != &rhs)
	{
		hostAddress = rhs.hostAddress;
		*addr4 = *rhs.addr4;
	}
	return *this;
}

//---------------------------------------------------------------------

Address & Address::operator = (const struct sockaddr_in & rhs)
{
	convertFromSockAddr(rhs);
	return *this;
}

//---------------------------------------------------------------------

void Address::convertFromSockAddr(const struct sockaddr_in & source)
{
	// extract IP bytes from ipv4add4
	const unsigned char *	ip;
	char	name[17] = {"\0"};

	ip = reinterpret_cast<const unsigned char *>(&source.sin_addr);
	snprintf(name, 17, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]); //lint !e534
	hostAddress = name;
	*addr4 = source;
}

//---------------------------------------------------------------------
/**
	@brief get a human readable host address

	Example:
	\code
	void foo(struct sockaddr_in & a)
	{
		Address b(a);
		printf("address = %%s\\n", b.getHostAddress().c_str());
	}
	\endcode

	@return A human readable host address string

	@author Justin Randall
*/
const std::string & Address::getHostAddress() const
{
	return hostAddress;
}

//---------------------------------------------------------------------
/**
	@brief get the port associated with this address

	Example:
	\code
	void foo(struct sockaddr_in & a)
	{
		Address b(a);
		printf("port = %%i\\n", b.getHostPort());
	}
	\endcode

	@return A human readable port in host-byte order associated with
		this address.

	@author Justin Randall
*/
const unsigned short Address::getHostPort() const
{
	return ntohs(addr4->sin_port);
}

//---------------------------------------------------------------------
/**
	@brief get the BSD sockaddr describing this address

	Example:
	\code
	void foo(SOCKET s, unsigned char * d, int l, const Address & a)
	{
		int t = sizeof(struct sockaddr_in);
		sendto(s, s, l, 0, reinterpret_cast<const struct sockaddr *>(&(a.getSockAddr4())), t);
	}
	\endcode

	@return a BSD sockaddr that describes this IPv4 address

	@author Justin Randall
*/
const struct sockaddr_in & Address::getSockAddr4() const
{
	return *addr4;
}

//---------------------------------------------------------------------
/**
	@brief equality operator

	The equality operator compares the ip address, ip port,
	and address family to establish equality.

	Example:
	\code
	Address a("127.0.0.1", 55443);
	Address b;

	b = a;
	assert(b == a);
	\endcode

	@return True of the right hand side is equal to this address

	@author Justin Randall
*/
const bool Address::operator == (const Address & rhs) const
{
	return (addr4->sin_addr.s_addr == rhs.addr4->sin_addr.s_addr &&
			addr4->sin_family == rhs.addr4->sin_family &&
			addr4->sin_port == rhs.addr4->sin_port);
}

//---------------------------------------------------------------------
/**
	@brief less-than comparison operator

	The < comparison operator compares the IP number and port. If
	the IP numbers are identical, but the left hand side port is
	less than the right hand side port, the operator will return
	true.

	@return true if the left hand side's IP number is less than
	the right hand side IP number. If the numbers are equal, it
	will return true if the left hand side IP port is less
	than the right hand side port. Otherwise it returns false.

	@author Justin Randall
*/
const bool Address::operator < (const Address & rhs) const
{
	return(addr4->sin_addr.s_addr < rhs.addr4->sin_addr.s_addr ||
		addr4->sin_addr.s_addr == rhs.addr4->sin_addr.s_addr &&
		addr4->sin_port < rhs.addr4->sin_port);
}

//---------------------------------------------------------------------
/**
	@brief inequality operator

	Leverages the equality operator, so whenever == returns true,
	this returns false, and visa versa.

	@return true if the right hand side is not equal to the left
	hand side. False if they are equal.

	@see Adress::operator==

	@author Justin Randall
*/
const bool Address::operator != (const Address & rhs) const
{
	return(! (rhs == *this));
}

//---------------------------------------------------------------------
/**
	@brief greater-than comparison operator

	The > comparison operator compares the IP number and port. If
	the IP numbers are identical, but the right hand side port is
	lesser than the left hand side port, the operator will return
	true.

	@return true if the left hand side's IP number is greater than
	the right hand side IP number. If the numbers are equal, it
	will return true if the left hand side IP port is greater
	than the right hand side port. Otherwise it returns false.

	@author Justin Randall
*/
const bool Address::operator > (const Address & rhs) const
{
	return(addr4->sin_addr.s_addr > rhs.addr4->sin_addr.s_addr ||
		addr4->sin_addr.s_addr == rhs.addr4->sin_addr.s_addr &&
		addr4->sin_port > rhs.addr4->sin_port);
}

//---------------------------------------------------------------------
/**
	@brief a hash_map support routine

	The STL hash_map (present in most STL implementations) requires
	a size_t return from a hash function to identify which bucket
	a particular value should reside in. On 32 bit or better platforms
	the sockaddr_in.sin_addr.s_addr member is small enough to
	qualify as a hash-result, provides reasonably unique values
	and is reproducable given an address input.

	Example:
	\code
	typedef std::hash_map<Address, Connection *, Address::HashFunction, Address::EqualFunction> AddressMap;
	\endcode

	@return the ip number member of a sockaddr_in struct

	@author Justin Randall
*/
size_t Address::hashFunction() const
{
	return addr4->sin_addr.s_addr;
}

//---------------------------------------------------------------------
/**
	@brief STL map support routine

	STL maps (including hash_maps) require unique keys, and therefore
	need to compare a key for equality with an existing target.

	The functor uses Address::operator = for the comparison.

	Example:
	\code
	typedef std::hash_map<Address, Connection *, Address::HashFunction, Address::EqualFunction> AddressMap;
	\endcode

	@return true if the left hand side and right hand side are equal
	using Address::operator =
	@see Address::operator=

*/
bool Address::EqualFunction::operator () (const Address & lhs, const Address & rhs) const
{
	return lhs == rhs;
}

//---------------------------------------------------------------------
/**
	@brief STL hash_map support routine

	The HashFunction::operator() invokes Address::hashFunction to
	determine an appropriate hash for the address.

	Example:
	\code
	typedef std::hash_map<Address, Connection *, Address::HashFunction, Address::EqualFunction> AddressMap;
	\endcode

	@see Address::hashFunction

	@author Justin Randall
*/
size_t Address::HashFunction::operator () (const Address & a) const
{
	return a.hashFunction();
}
//---------------------------------------------------------------------



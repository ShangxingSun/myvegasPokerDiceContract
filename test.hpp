#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#define EOS_SYMBOL symbol("EOS", 4)


using namespace eosio;
using namespace std;

CONTRACT test : public eosio::contract
{
	public:
	test(name receiver, name code, datastream<const char *> ds) 
		: contract(receiver, code, ds) {}

	struct init
	{
		init(){};
		eosio::name name;
		EOSLIB_SERIALIZE(init, (name))
	};

	ACTION init(eosio::name name);
	void transfer(
			eosio::name from,
			eosio::name to,
			eosio::asset quantity,
			std::string memo
			);
};

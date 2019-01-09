#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/privileged.h>
#include <set>
#include <unordered_map>

using namespace eosio;
using std::string;

#define EOS_SYMBOL symbol("EOS", 4)


using namespace eosio;
using namespace std;

CONTRACT pokerrollcontract : public eosio::contract
{
	public:
	pokerrollcontract(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds),nonces(_self, _self.value), pokerdicepools(_self, _self.value) {}

	struct st_pdpool
	{
		name owner;
		string betcurrency;
		uint32_t nonce;
		uint64_t totalbet;
		string userseed;
		string bet_cards;
		string bet_value;

		uint64_t primary_key() const { return owner.value; }
		EOSLIB_SERIALIZE(st_pdpool, (owner)(nonce)(totalbet)(userseed)(bet_cards)(bet_value))
	};

	struct st_nonces{
		name owner;
		uint32_t number;

		uint64_t primary_key() const { return owner.value; }
		EOSLIB_SERIALIZE(st_nonces, (owner)(number))
	};

	typedef eosio::multi_index<"pdpools"_n, st_pdpool> _pdpools;
	_pdpools pokerdicepools;

	typedef multi_index<"nonces"_n, st_nonces> _tb_nonces;
	_tb_nonces nonces;

	ACTION pdreceipt(string game_id, const name player, string game, string seed, string bet_result,
		string bet_cards, string bet_value, uint64_t betnum, uint64_t winnum, string token, string pub_key);

	void transfer(
			eosio::name from,
			eosio::name to,
			eosio::asset quantity,
			std::string memo
			);
	
	uint32_t increment_nonce(const name user);
	void sanity_check(uint64_t code, name act);
};

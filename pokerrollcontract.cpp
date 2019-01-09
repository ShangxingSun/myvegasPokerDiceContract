#include "./pokerrollcontract.hpp"





void pokerrollcontract::transfer(name from, name to, asset t, string memo)
{
	// No bet from eosvegascoin
	if (from == name("eosvegascoin") || from == name("eosvegascorp") || from == name("eosvegasopmk"))
	{
		return;
	}

	string bettoken = "EOS";
	eosio_assert(_code == name("eosio.token"), "EOS should be sent by eosio.token");
	eosio_assert(t.symbol == EOS_SYMBOL, "Incorrect token type.");
	sanity_check(name("eosio.token").value, name("transfer"));


	eosio_assert(to == _self, "Transfer not made to this contract");
	eosio_assert(t.symbol.is_valid(), "Invalid token symbol");
	eosio_assert(t.amount > 0, "Quantity must be positive");

	string usercomment = memo;

	uint32_t gameid = 0;

	uint32_t typeidx = usercomment.find("type[");
	if (typeidx > 0 && typeidx != 4294967295)
	{
		uint32_t pos = usercomment.find("]");
		if (pos > 0 && pos != 4294967295)
		{
			string ucm = usercomment.substr(typeidx + 5, pos - typeidx - 5);
			gameid = stoi(ucm);
		}
	}

	string userseed = "";
	uint32_t seedidx = usercomment.find("seed[");
	if (seedidx > 0 && seedidx != 4294967295)
	{
		uint32_t pos = usercomment.find("]", seedidx);
		if (pos > 0 && pos != 4294967295)
		{
			userseed = usercomment.substr(seedidx + 5, pos - seedidx - 5);
		}
		else{
			userseed = "-1";
		}

	}else{
		userseed = "-2";
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////unique section///////////////////////////////////////////////

	// gameid =5 is for dicePoker
	eosio_assert((gameid == 0 || gameid == 1 || gameid == 2 || gameid == 3 || gameid == 5), "Non-recognized game id");


	/* //just ignore metadatas for a while
			// metadatas[1] is blackjack, metadatas[2] is video poker.
			auto itr_metadata = gameid == 2 ? metadatas.find(1) : metadatas.find(2);
			auto itr_metadata2 = metadatas.find(0);

			eosio_assert(itr_metadata != metadatas.end(), "Game is not found.");
			eosio_assert(itr_metadata2 != metadatas.end(), "No game is found.");

			eosio_assert(itr_metadata2->gameon == 1 || t.from == N(blockfishbgp), "All games are temporarily paused.");
			eosio_assert(itr_metadata->gameon == 1 || t.from == N(blockfishbgp), "Game is temporarily paused.");

	*/

	if (gameid == 5)
	{
		eosio_assert(userseed.length() > 0, "user seed cannot by empty.");

		////////////////bet_cards -> the cards user placed chips on/////////////////////////////
		////////////////bet_value -> the value on each card/////////////////////////////////////
		string bet_cards = "";
		string bet_value = "";

		uint32_t betcardidx = usercomment.find("bet_cards[");
		if (betcardidx > 0 && betcardidx != 4294967295)
		{
			uint32_t betcardpos = usercomment.find("]", betcardidx);
			if (betcardpos > 0 && betcardpos != 4294967295)
			{
				string bet_cards = usercomment.substr(betcardidx + 10, betcardpos - betcardidx - 10);
			}
		}
		uint32_t valueidx = usercomment.find("bet_value[");
		if (valueidx > 0 && valueidx != 4294967295)
		{
			uint32_t valuepos = usercomment.find("]", valueidx);
			if (valuepos > 0 && valuepos != 4294967295)
			{
				string bet_value = usercomment.substr(valueidx + 10, valuepos - valueidx - 10);
			}
		}

		//////////////////////////////need rewrite///////////////////////////////////////////////
		uint32_t nonce = increment_nonce(from);

		eosio_assert(t.amount >= 1000, "PokerDice: Below minimum bet threshold!");
		eosio_assert(t.amount <= 200000, "PokerDice:Exceeds bet cap!");


		auto itr_pdpools = pokerdicepools.find(from.value);

		eosio_assert(itr_pdpools == pokerdicepools.end(), "pokerdice: your last round is not finished. Please contact admin!");

		//  	    name owner;
		// 		    uint32_t betcurrency;
		//			uint32_t nonce;
		//       	uint64_t totalbet;
		//       	string userseed;
		//       	string bet_cards;
		//     	 	string bet_value;

		pokerdicepools.emplace(_self, [&](auto &p) {
			p.owner = from;
			p.betcurrency = bettoken;
			p.nonce = nonce;
			p.totalbet = t.amount;
			p.userseed = userseed;
			p.bet_cards = bet_cards;
			p.bet_value = bet_value;
		});
	}
}


void pokerrollcontract::pdreceipt(string game_id, const name player, string game, string seed, string bet_result,
	string bet_cards, string bet_value, uint64_t betnum, uint64_t winnum, string token, string pub_key) {
	
	
	require_auth(name("eosvegasjack"));
	require_recipient(player);

	auto itr_pdpools = pokerdicepools.find(player.value);

	eosio_assert(itr_pdpools != pokerdicepools.end(), "PokerDice: user pool not found");
	eosio_assert(itr_pdpools->totalbet > 0, "PokerDice:lackjack:bet must be larger than zero");
	eosio_assert(itr_pdpools->totalbet == betnum, "PokerDice:totalbet must be equal to betnum");

	int pos1 = seed.find("_");
	eosio_assert(pos1 > 0, "PokerDice: seed is incorrect.");
	string ucm = seed.substr(0, pos1);
	uint32_t seednonce = stoi(ucm);
	eosio_assert(seednonce == itr_pdpools->nonce, "PokerDice: nonce does not match.");

	eosio_assert(token == itr_pdpools->betcurrency, "PokerDice: bet token does not match.");

	if (token == "EOS")
	{
		/*
		//paccounts  is exp related, ignore for now
		auto itr_paccount = paccounts.find(player);
		eosio_assert(itr_paccount != paccounts.end(), "PokerDice: user not found");
		*/
		asset bal = asset((winnum), EOS_SYMBOL);
		if (bal.amount > 0)
		{
			// withdraw
			action(permission_level{ _self, name("active") }, name("eosio.token"),
				name("transfer"), std::make_tuple(_self, player, bal, std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- PokerDice.rovegas.com")))
				.send();
		}
	}

	pokerdicepools.erase(itr_pdpools);
}

void pokerrollcontract::sanity_check(uint64_t code, name act)
{
	char buffer[32];

	for (int cnt = 0; cnt < 1; cnt++)
	{
		uint32_t na = get_action(1, cnt, buffer, 0);

		if (na == -1)
			return;
		eosio_assert(cnt == 0, "Invalid request!");

		action a = get_action(1, cnt);
		eosio_assert(a.account.value == code, "Invalid request!");
		eosio_assert(a.name == act, "Invalid request!");
		auto v = a.authorization;
		eosio_assert(v.size() == 1, "Invalid request!!");

		permission_level pl = v[0];
		eosio_assert(pl.permission == name("active") || pl.permission == name("owner"), "Invalid request!!");
	}
}

uint32_t pokerrollcontract::increment_nonce(const name user)
{
	// Get current nonce and increment it
	uint32_t nonce = 0;
	auto itr_nonce = nonces.find(user.value);
	if (itr_nonce != nonces.end())
	{
		nonce = itr_nonce->number;
	}
	if (itr_nonce == nonces.end())
	{
		nonces.emplace(_self, [&](auto &p) {
			p.owner = name{ user };
			p.number = 1;
		});
	}
	else
	{
		nonces.modify(itr_nonce, _self, [&](auto &p) {
			p.number += 1;
		});
	}
	return nonce;
}

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
	if (code == "eosio.token"_n.value && action == "transfer"_n.value)
	{
		eosio::execute_action(
				eosio::name(receiver), eosio::name(code), &pokerrollcontract::transfer
				);
	}
	else if (code == receiver)
	{
		switch (action)
		{
			EOSIO_DISPATCH_HELPER(pokerrollcontract, (pdreceipt))
		}
	}
}

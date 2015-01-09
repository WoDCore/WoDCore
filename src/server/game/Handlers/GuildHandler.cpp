/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "GossipDef.h"
#include "SocialMgr.h"
#include "GuildPackets.h"

void WorldSession::HandleGuildQueryOpcode(WorldPackets::Guild::QueryGuildInfo& query)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_QUERY [%s]: Guild: %s Target: %s",
        GetPlayerInfo().c_str(), query.GuildGuid.ToString().c_str(), query.PlayerGuid.ToString().c_str());

    if (Guild* guild = sGuildMgr->GetGuildByGuid(query.GuildGuid))
        if (guild->IsMember(query.PlayerGuid))
        {
            guild->SendQueryResponse(this);
            return;
        }

    WorldPackets::Guild::QueryGuildInfoResponse response;
    response.GuildGuid = query.GuildGuid;
    SendPacket(response.Write());

    TC_LOG_DEBUG("guild", "SMSG_GUILD_QUERY_RESPONSE [%s]", GetPlayerInfo().c_str());
}

void WorldSession::HandleGuildInviteOpcode(WorldPackets::Guild::GuildInviteByName& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_INVITE [%s]: Invited: %s", GetPlayerInfo().c_str(), packet.Name.c_str());
    if (normalizePlayerName(packet.Name))
        if (Guild* guild = GetPlayer()->GetGuild())
            guild->HandleInviteMember(this, packet.Name);
}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recvPacket)
{
    ObjectGuid playerGuid;

    playerGuid[6] = recvPacket.ReadBit();
    playerGuid[5] = recvPacket.ReadBit();
    playerGuid[4] = recvPacket.ReadBit();
    playerGuid[0] = recvPacket.ReadBit();
    playerGuid[1] = recvPacket.ReadBit();
    playerGuid[3] = recvPacket.ReadBit();
    playerGuid[7] = recvPacket.ReadBit();
    playerGuid[2] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(playerGuid[2]);
    recvPacket.ReadByteSeq(playerGuid[6]);
    recvPacket.ReadByteSeq(playerGuid[5]);
    recvPacket.ReadByteSeq(playerGuid[7]);
    recvPacket.ReadByteSeq(playerGuid[1]);
    recvPacket.ReadByteSeq(playerGuid[4]);
    recvPacket.ReadByteSeq(playerGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[0]);

    TC_LOG_DEBUG("guild", "CMSG_GUILD_REMOVE [%s]: Target: %s", GetPlayerInfo().c_str(), playerGuid.ToString().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleRemoveMember(this, playerGuid);
}

void WorldSession::HandleGuildAcceptInvite(WorldPackets::Guild::AcceptGuildInvite& /*invite*/)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_ACCEPT_INVITE [%s]", GetPlayerInfo().c_str());

    if (!GetPlayer()->GetGuildId())
        if (Guild* guild = sGuildMgr->GetGuildById(GetPlayer()->GetGuildIdInvited()))
            guild->HandleAcceptMember(this);
}

void WorldSession::HandleGuildDeclineInvitation(WorldPackets::Guild::GuildDeclineInvitation& /*decline*/)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_DECLINE_INVITATION [%s]", GetPlayerInfo().c_str());

    GetPlayer()->SetGuildIdInvited(UI64LIT(0));
    GetPlayer()->SetInGuild(UI64LIT(0));
}

void WorldSession::HandleGuildGetRoster(WorldPackets::Guild::GuildGetRoster& /*packet*/)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_ROSTER [%s]", GetPlayerInfo().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleRoster(this);
    else
        Guild::SendCommandResult(this, GUILD_COMMAND_ROSTER, ERR_GUILD_PLAYER_NOT_IN_GUILD);
}

void WorldSession::HandleGuildPromoteMember(WorldPackets::Guild::GuildPromoteMember& promote)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_PROMOTE [%s]: Target: %s", GetPlayerInfo().c_str(), promote.Promotee.ToString().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleUpdateMemberRank(this, promote.Promotee, false);
}

void WorldSession::HandleGuildDemoteMember(WorldPackets::Guild::GuildDemoteMember& demote)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_DEMOTE [%s]: Target: %s", GetPlayerInfo().c_str(), demote.Demotee.ToString().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleUpdateMemberRank(this, demote.Demotee, true);
}

void WorldSession::HandleGuildAssignRank(WorldPackets::Guild::GuildAssignMemberRank& packet)
{
    ObjectGuid setterGuid = GetPlayer()->GetGUID();

    TC_LOG_DEBUG("guild", "CMSG_GUILD_ASSIGN_MEMBER_RANK [%s]: Target: %s Rank: %u, Issuer: %s",
        GetPlayerInfo().c_str(), packet.Member.ToString().c_str(), packet.RankOrder, setterGuid.ToString().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetMemberRank(this, packet.Member, setterGuid, packet.RankOrder);
}

void WorldSession::HandleGuildLeave(WorldPackets::Guild::GuildLeave& /*leave*/)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_LEAVE [%s]", GetPlayerInfo().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleLeaveMember(this);
}

void WorldSession::HandleGuildDelete(WorldPackets::Guild::GuildDelete& /*packet*/)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_DISBAND [%s]", GetPlayerInfo().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleDelete(this);
}

void WorldSession::HandleGuildUpdateMotdText(WorldPackets::Guild::GuildUpdateMotdText& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_MOTD [%s]: MOTD: %s", GetPlayerInfo().c_str(), packet.MotdText.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetMOTD(this, packet.MotdText);
}

void WorldSession::HandleGuildSetNoteOpcode(WorldPacket& recvPacket)
{
    ObjectGuid playerGuid;

    playerGuid[1] = recvPacket.ReadBit();
    playerGuid[4] = recvPacket.ReadBit();
    playerGuid[5] = recvPacket.ReadBit();
    playerGuid[3] = recvPacket.ReadBit();
    playerGuid[0] = recvPacket.ReadBit();
    playerGuid[7] = recvPacket.ReadBit();
    bool ispublic = recvPacket.ReadBit();      // 0 == Officer, 1 == Public
    playerGuid[6] = recvPacket.ReadBit();
    uint32 noteLength = recvPacket.ReadBits(8);
    playerGuid[2] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(playerGuid[4]);
    recvPacket.ReadByteSeq(playerGuid[5]);
    recvPacket.ReadByteSeq(playerGuid[0]);
    recvPacket.ReadByteSeq(playerGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[1]);
    recvPacket.ReadByteSeq(playerGuid[6]);
    recvPacket.ReadByteSeq(playerGuid[7]);
    std::string note = recvPacket.ReadString(noteLength);
    recvPacket.ReadByteSeq(playerGuid[2]);

    TC_LOG_DEBUG("guild", "CMSG_GUILD_SET_NOTE [%s]: Target: %s, Note: %s, Public: %u",
        GetPlayerInfo().c_str(), playerGuid.ToString().c_str(), note.c_str(), ispublic);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetMemberNote(this, note, playerGuid, ispublic);
}

void WorldSession::HandleGuildGetRanks(WorldPackets::Guild::GuildGetRanks& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_QUERY_RANKS [%s]: Guild: %s",
        GetPlayerInfo().c_str(), packet.GuildGUID.ToString().c_str());

    if (Guild* guild = sGuildMgr->GetGuildByGuid(packet.GuildGUID))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildRankInfo(this);
}

void WorldSession::HandleGuildAddRank(WorldPackets::Guild::GuildAddRank& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_ADD_RANK [%s]: Rank: %s", GetPlayerInfo().c_str(), packet.Name.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleAddNewRank(this, packet.Name);
}

void WorldSession::HandleGuildDelRank(WorldPackets::Guild::GuildDeleteRank& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_DEL_RANK [%s]: Rank: %u", GetPlayerInfo().c_str(), packet.RankOrder);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleRemoveRank(this, packet.RankOrder);
}

void WorldSession::HandleGuildChangeInfoText(WorldPackets::Guild::GuildUpdateInfoText& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_INFO_TEXT [%s]: %s", GetPlayerInfo().c_str(), packet.InfoText.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetInfo(this, packet.InfoText);
}

void WorldSession::HandleSaveGuildEmblemOpcode(WorldPacket& recvPacket)
{
    ObjectGuid vendorGuid;
    recvPacket >> vendorGuid;

    EmblemInfo emblemInfo;
    emblemInfo.ReadPacket(recvPacket);

    TC_LOG_DEBUG("guild", "MSG_SAVE_GUILD_EMBLEM [%s]: Guid: [%s] Style: %d, Color: %d, BorderStyle: %d, BorderColor: %d, BackgroundColor: %d"
        , GetPlayerInfo().c_str(), vendorGuid.ToString().c_str(), emblemInfo.GetStyle()
        , emblemInfo.GetColor(), emblemInfo.GetBorderStyle()
        , emblemInfo.GetBorderColor(), emblemInfo.GetBackgroundColor());

    if (GetPlayer()->GetNPCIfCanInteractWith(vendorGuid, UNIT_NPC_FLAG_TABARDDESIGNER))
    {
        // Remove fake death
        if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
            GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

        if (Guild* guild = GetPlayer()->GetGuild())
            guild->HandleSetEmblem(this, emblemInfo);
        else
            Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_NOGUILD); // "You are not part of a guild!";
    }
    else
        Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_INVALIDVENDOR); // "That's not an emblem vendor!"
}

void WorldSession::HandleGuildEventLogQuery(WorldPackets::Guild::GuildEventLogQuery& /*packet*/)
{
    TC_LOG_DEBUG("guild", "MSG_GUILD_EVENT_LOG_QUERY [%s]", GetPlayerInfo().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendEventLog(this);
}

void WorldSession::HandleGuildBankMoneyWithdrawn(WorldPackets::Guild::GuildBankRemainingWithdrawMoneyQuery& /*packet*/)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_MONEY_WITHDRAWN [%s]", GetPlayerInfo().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendMoneyInfo(this);
}

void WorldSession::HandleGuildPermissions(WorldPackets::Guild::GuildPermissionsQuery& /* packet */)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_PERMISSIONS [%s]", GetPlayerInfo().c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendPermissions(this);
}

// Called when clicking on Guild bank gameobject
void WorldSession::HandleGuildBankerActivate(WorldPackets::Guild::GuildBankActivate& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANKER_ACTIVATE [%s]: [%s] AllSlots: %u"
        , GetPlayerInfo().c_str(), packet.Banker.ToString().c_str(), packet.FullUpdate);

    GameObject const* const go = GetPlayer()->GetGameObjectIfCanInteractWith(packet.Banker, GAMEOBJECT_TYPE_GUILD_BANK);
    if (!go)
        return;

    Guild* const guild = GetPlayer()->GetGuild();
    if (!guild)
    {
        Guild::SendCommandResult(this, GUILD_COMMAND_VIEW_TAB, ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    guild->SendBankList(this, 0, packet.FullUpdate);
}

// Called when opening guild bank tab only (first one)
void WorldSession::HandleGuildBankQueryTab(WorldPackets::Guild::GuildBankQueryTab& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_QUERY_TAB [%s]: %s, TabId: %u, ShowTabs: %u"
        , GetPlayerInfo().c_str(), packet.Banker.ToString().c_str(), packet.Tab, packet.FullUpdate);

    if (GetPlayer()->GetGameObjectIfCanInteractWith(packet.Banker, GAMEOBJECT_TYPE_GUILD_BANK))
        if (Guild* guild = GetPlayer()->GetGuild())
            guild->SendBankList(this, packet.Tab, packet.FullUpdate);
}

void WorldSession::HandleGuildBankDepositMoney(WorldPackets::Guild::GuildBankDepositMoney& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_DEPOSIT_MONEY [%s]: [%s], money: " UI64FMTD,
        GetPlayerInfo().c_str(), packet.Banker.ToString().c_str(), packet.Money);

    if (GetPlayer()->GetGameObjectIfCanInteractWith(packet.Banker, GAMEOBJECT_TYPE_GUILD_BANK))
        if (packet.Money && GetPlayer()->HasEnoughMoney(packet.Money))
            if (Guild* guild = GetPlayer()->GetGuild())
                guild->HandleMemberDepositMoney(this, packet.Money);
}

void WorldSession::HandleGuildBankWithdrawMoney(WorldPackets::Guild::GuildBankWithdrawMoney& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_WITHDRAW_MONEY [%s]: [%s], money: " UI64FMTD,
        GetPlayerInfo().c_str(), packet.Banker.ToString().c_str(), packet.Money);

    if (packet.Money && GetPlayer()->GetGameObjectIfCanInteractWith(packet.Banker, GAMEOBJECT_TYPE_GUILD_BANK))
        if (Guild* guild = GetPlayer()->GetGuild())
            guild->HandleMemberWithdrawMoney(this, packet.Money);
}

void WorldSession::HandleGuildBankSwapItems(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_SWAP_ITEMS [%s]", GetPlayerInfo().c_str());

    ObjectGuid GoGuid;
    recvPacket >> GoGuid;

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(GoGuid, GAMEOBJECT_TYPE_GUILD_BANK))
    {
        recvPacket.rfinish();                   // Prevent additional spam at rejected packet
        return;
    }

    Guild* guild = GetPlayer()->GetGuild();
    if (!guild)
    {
        recvPacket.rfinish();                   // Prevent additional spam at rejected packet
        return;
    }

    uint8 bankToBank;
    recvPacket >> bankToBank;

    uint8 tabId;
    uint8 slotId;
    uint32 itemEntry;
    uint32 splitedAmount = 0;

    if (bankToBank)
    {
        uint8 destTabId;
        recvPacket >> destTabId;

        uint8 destSlotId;
        recvPacket >> destSlotId;

        uint32 destItemEntry;
        recvPacket >> destItemEntry;

        recvPacket >> tabId;
        recvPacket >> slotId;
        recvPacket >> itemEntry;
        recvPacket.read_skip<uint8>();                       // Always 0
        recvPacket >> splitedAmount;

        guild->SwapItems(GetPlayer(), tabId, slotId, destTabId, destSlotId, splitedAmount);
    }
    else
    {
        uint8 playerBag = NULL_BAG;
        uint8 playerSlotId = NULL_SLOT;
        uint8 toChar = 1;

        recvPacket >> tabId;
        recvPacket >> slotId;
        recvPacket >> itemEntry;

        uint8 autoStore;
        recvPacket >> autoStore;
        if (autoStore)
        {
            recvPacket.read_skip<uint32>();                  // autoStoreCount
            recvPacket.read_skip<uint8>();                   // ToChar (?), always and expected to be 1 (autostore only triggered in Bank -> Char)
            recvPacket.read_skip<uint32>();                  // Always 0
        }
        else
        {
            recvPacket >> playerBag;
            recvPacket >> playerSlotId;
            recvPacket >> toChar;
            recvPacket >> splitedAmount;
        }

        // Player <-> Bank
        // Allow to work with inventory only
        if (!Player::IsInventoryPos(playerBag, playerSlotId) && !(playerBag == NULL_BAG && playerSlotId == NULL_SLOT))
            GetPlayer()->SendEquipError(EQUIP_ERR_INTERNAL_BAG_ERROR, NULL);
        else
            guild->SwapItemsWithInventory(GetPlayer(), toChar != 0, tabId, slotId, playerBag, playerSlotId, splitedAmount);
    }
}

void WorldSession::HandleGuildBankBuyTab(WorldPackets::Guild::GuildBankBuyTab& packet)
{
    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_BUY_TAB [%s]: [%s[, TabId: %u", GetPlayerInfo().c_str(), packet.Banker.ToString().c_str(), packet.BankTab);

    if (!packet.Banker || GetPlayer()->GetGameObjectIfCanInteractWith(packet.Banker, GAMEOBJECT_TYPE_GUILD_BANK))
        if (Guild* guild = GetPlayer()->GetGuild())
            guild->HandleBuyBankTab(this, packet.BankTab);
}

void WorldSession::HandleGuildBankUpdateTab(WorldPacket& recvPacket)
{
    ObjectGuid guid;
    uint8 tabId;
    std::string name, icon;

    recvPacket >> guid >> tabId >> name >> icon;

    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_UPDATE_TAB [%s]: [%s], TabId: %u, Name: %s, Icon: %s"
        , GetPlayerInfo().c_str(), guid.ToString().c_str(), tabId, name.c_str(), icon.c_str());
    if (!name.empty() && !icon.empty())
        if (GetPlayer()->GetGameObjectIfCanInteractWith(guid, GAMEOBJECT_TYPE_GUILD_BANK))
            if (Guild* guild = GetPlayer()->GetGuild())
                guild->HandleSetBankTabInfo(this, tabId, name, icon);
}

void WorldSession::HandleGuildBankLogQuery(WorldPacket& recvPacket)
{
    uint32 tabId;
    recvPacket >> tabId;

    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_LOG_QUERY [%s]: TabId: %u", GetPlayerInfo().c_str(), tabId);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendBankLog(this, tabId);
}

void WorldSession::HandleQueryGuildBankTabText(WorldPacket &recvPacket)
{
    uint8 tabId;
    recvPacket >> tabId;

    TC_LOG_DEBUG("guild", "CMSG_GUILD_BANK_QUERY_TEXT [%s]: TabId: %u", GetPlayerInfo().c_str(), tabId);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendBankTabText(this, tabId);
}

void WorldSession::HandleSetGuildBankTabText(WorldPacket& recvPacket)
{
    uint32 tabId;
    recvPacket >> tabId;

    uint32 textLen = recvPacket.ReadBits(14);
    std::string text = recvPacket.ReadString(textLen);

    TC_LOG_DEBUG("guild", "CMSG_SET_GUILD_BANK_TEXT [%s]: TabId: %u, Text: %s", GetPlayerInfo().c_str(), tabId, text.c_str());

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SetBankTabText(tabId, text);
}

void WorldSession::HandleGuildQueryXPOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUERY_GUILD_XP");

    ObjectGuid guildGuid;

    guildGuid[2] = recvPacket.ReadBit();
    guildGuid[1] = recvPacket.ReadBit();
    guildGuid[0] = recvPacket.ReadBit();
    guildGuid[5] = recvPacket.ReadBit();
    guildGuid[4] = recvPacket.ReadBit();
    guildGuid[7] = recvPacket.ReadBit();
    guildGuid[6] = recvPacket.ReadBit();
    guildGuid[3] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(guildGuid[7]);
    recvPacket.ReadByteSeq(guildGuid[2]);
    recvPacket.ReadByteSeq(guildGuid[3]);
    recvPacket.ReadByteSeq(guildGuid[6]);
    recvPacket.ReadByteSeq(guildGuid[1]);
    recvPacket.ReadByteSeq(guildGuid[5]);
    recvPacket.ReadByteSeq(guildGuid[0]);
    recvPacket.ReadByteSeq(guildGuid[4]);

    TC_LOG_DEBUG("guild", "CMSG_QUERY_GUILD_XP [%s]: Guild: %s", GetPlayerInfo().c_str(), guildGuid.ToString().c_str());

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildXP(this);
}

void WorldSession::HandleGuildSetRankPermissionsOpcode(WorldPacket& recvPacket)
{
    Guild* guild = GetPlayer()->GetGuild();
    if (!guild)
    {
        recvPacket.rfinish();
        return;
    }

    uint32 oldRankId;
    uint32 newRankId;
    uint32 oldRights;
    uint32 newRights;
    uint32 moneyPerDay;

    recvPacket >> oldRankId;
    recvPacket >> oldRights;
    recvPacket >> newRights;

    GuildBankRightsAndSlotsVec rightsAndSlots(GUILD_BANK_MAX_TABS);
    for (uint8 tabId = 0; tabId < GUILD_BANK_MAX_TABS; ++tabId)
    {
        uint32 bankRights;
        uint32 slots;

        recvPacket >> bankRights;
        recvPacket >> slots;

        rightsAndSlots[tabId] = GuildBankRightsAndSlots(tabId, uint8(bankRights), slots);
    }

    recvPacket >> moneyPerDay;
    recvPacket >> newRankId;
    uint32 nameLength = recvPacket.ReadBits(7);
    std::string rankName = recvPacket.ReadString(nameLength);

    TC_LOG_DEBUG("guild", "CMSG_GUILD_SET_RANK_PERMISSIONS [%s]: Rank: %s (%u)", GetPlayerInfo().c_str(), rankName.c_str(), newRankId);

    guild->HandleSetRankInfo(this, newRankId, rankName, newRights, moneyPerDay, rightsAndSlots);
}

void WorldSession::HandleGuildRequestPartyState(WorldPackets::Guild::RequestGuildPartyState& packet)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_GUILD_REQUEST_PARTY_STATE");

    if (Guild* guild = sGuildMgr->GetGuildByGuid(packet.GuildGUID))
        guild->HandleGuildPartyRequest(this);
}

void WorldSession::HandleGuildRequestChallengeUpdate(WorldPacket& /*recvPacket*/)
{
    if (Guild* guild = _player->GetGuild())
        guild->HandleGuildRequestChallengeUpdate(this);
}

void WorldSession::HandleGuildRequestMaxDailyXP(WorldPacket& recvPacket)
{
    ObjectGuid guid;
    guid[0] = recvPacket.ReadBit();
    guid[3] = recvPacket.ReadBit();
    guid[5] = recvPacket.ReadBit();
    guid[1] = recvPacket.ReadBit();
    guid[4] = recvPacket.ReadBit();
    guid[6] = recvPacket.ReadBit();
    guid[7] = recvPacket.ReadBit();
    guid[2] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(guid[7]);
    recvPacket.ReadByteSeq(guid[4]);
    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[5]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[0]);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guid))
    {
        if (guild->IsMember(_player->GetGUID()))
        {
            WorldPacket data(SMSG_GUILD_MAX_DAILY_XP, 8);
            data << uint64(sWorld->getIntConfig(CONFIG_GUILD_DAILY_XP_CAP));
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleAutoDeclineGuildInvites(WorldPacket& recvPacket)
{
    uint8 enable;
    recvPacket >> enable;

    GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_AUTO_DECLINE_GUILD, enable != 0);
}

void WorldSession::HandleRequestGuildRewardsList(WorldPackets::Guild::RequestGuildRewardsList& packet)
{
    if (sGuildMgr->GetGuildById(_player->GetGuildId()))
    {
        std::vector<GuildReward> const& rewards = sGuildMgr->GetGuildRewards();

        WorldPackets::Guild::GuildRewardList rewardList;
        rewardList.Version = uint32(time(NULL));
        rewardList.RewardItems.reserve(rewards.size());

        for (uint32 i = 0; i < rewards.size(); i++)
        {
            WorldPackets::Guild::GuildRewardItem rewardItem;
            rewardItem.ItemID = rewards[i].ItemID;
            rewardItem.RaceMask = rewards[i].RaceMask;
            rewardItem.MinGuildLevel = 0;
            rewardItem.MinGuildRep = rewards[i].MinGuildRep;
            rewardItem.AchievementsRequired = rewards[i].AchievementsRequired;
            rewardItem.Cost = rewards[i].Cost;
            rewardList.RewardItems.push_back(rewardItem);
        }

        SendPacket(rewardList.Write());
    }
}

void WorldSession::HandleGuildQueryNewsOpcode(WorldPacket& recvPacket)
{
    recvPacket.read_skip<uint32>();
    TC_LOG_DEBUG("guild", "CMSG_GUILD_QUERY_NEWS [%s]", GetPlayerInfo().c_str());
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->SendNewsUpdate(this);
}

void WorldSession::HandleGuildNewsUpdateStickyOpcode(WorldPacket& recvPacket)
{
    uint32 newsId;
    bool sticky;
    ObjectGuid guid;

    recvPacket >> newsId;

    guid[2] = recvPacket.ReadBit();
    guid[4] = recvPacket.ReadBit();
    guid[3] = recvPacket.ReadBit();
    guid[0] = recvPacket.ReadBit();
    sticky = recvPacket.ReadBit();
    guid[6] = recvPacket.ReadBit();
    guid[7] = recvPacket.ReadBit();
    guid[1] = recvPacket.ReadBit();
    guid[5] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[0]);
    recvPacket.ReadByteSeq(guid[5]);
    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[7]);
    recvPacket.ReadByteSeq(guid[4]);

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleNewsSetSticky(this, newsId, sticky);
}

void WorldSession::HandleGuildSetGuildMaster(WorldPacket& recvPacket)
{
    uint8 nameLength = recvPacket.ReadBits(7);
    // This is related to guild master inactivity.
    /*bool isDethrone = */recvPacket.ReadBit();
    std::string playerName = recvPacket.ReadString(nameLength);
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetNewGuildMaster(this, playerName);
}

void WorldSession::HandleGuildSetAchievementTracking(WorldPacket& recvPacket)
{
    uint32 count = recvPacket.ReadBits(24);
    std::set<uint32> achievementIds;

    for (uint32 i = 0; i < count; ++i)
    {
        uint32 achievementId;
        recvPacket >> achievementId;
        achievementIds.insert(achievementId);
    }

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetAchievementTracking(this, achievementIds);
}

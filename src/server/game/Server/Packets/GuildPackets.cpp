/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "GuildPackets.h"

void WorldPackets::Guild::QueryGuildInfo::Read()
{
    _worldPacket >> GuildGuid;
    _worldPacket >> PlayerGuid;
}

WorldPackets::Guild::QueryGuildInfoResponse::QueryGuildInfoResponse()
    : ServerPacket(SMSG_GUILD_QUERY_RESPONSE) { }

WorldPacket const* WorldPackets::Guild::QueryGuildInfoResponse::Write()
{
    _worldPacket << GuildGuid;
    _worldPacket.WriteBit(Info.HasValue);

    if (Info.HasValue)
    {
        _worldPacket << Info.Value.GuildGUID;
        _worldPacket << uint32(Info.Value.VirtualRealmAddress);
        _worldPacket << uint32(Info.Value.Ranks.size());
        _worldPacket << uint32(Info.Value.EmblemStyle);
        _worldPacket << uint32(Info.Value.EmblemColor);
        _worldPacket << uint32(Info.Value.BorderStyle);
        _worldPacket << uint32(Info.Value.BorderColor);
        _worldPacket << uint32(Info.Value.BackgroundColor);

        for (GuildInfo::GuildInfoRank const& rank : Info.Value.Ranks)
        {
            _worldPacket << uint32(rank.RankID);
            _worldPacket << uint32(rank.RankOrder);

            _worldPacket.WriteBits(rank.RankName.size(), 7);
            _worldPacket.WriteString(rank.RankName);
        }

        _worldPacket.WriteBits(Info.Value.GuildName.size(), 7);
        _worldPacket.WriteString(Info.Value.GuildName);
    }
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildRoster::Write()
{
    _worldPacket << NumAccounts;
    _worldPacket.AppendPackedTime(CreateDate);
    _worldPacket << MaxWeeklyRep;
    _worldPacket << uint32(MemberData.size());

    for (GuildRosterMemberData const& member : MemberData)
        _worldPacket << member;

    _worldPacket.ResetBitPos();
    _worldPacket.WriteBits(WelcomeText.length(), 10);
    _worldPacket.WriteBits(InfoText.length(), 10);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(WelcomeText);
    _worldPacket.WriteString(InfoText);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildRosterUpdate::Write()
{
    _worldPacket << uint32(MemberData.size());

    for (GuildRosterMemberData const& member : MemberData)
        _worldPacket << member;

    return &_worldPacket;
}

void WorldPackets::Guild::GuildUpdateMotdText::Read()
{
    uint32 textLen = _worldPacket.ReadBits(10);
    MotdText = _worldPacket.ReadString(textLen);
}

WorldPacket const* WorldPackets::Guild::GuildCommandResult::Write()
{
    _worldPacket << Command;
    _worldPacket << Result;

    _worldPacket.WriteBits(Name.length(), 8);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

void WorldPackets::Guild::GuildInviteByName::Read()
{
    uint32 nameLen = _worldPacket.ReadBits(9);
    Name = _worldPacket.ReadString(nameLen);
}

WorldPacket const* WorldPackets::Guild::GuildInvite::Write()
{
    _worldPacket.WriteBits(InviterName.length(), 6);
    _worldPacket.WriteBits(OldGuildName.length(), 7);
    _worldPacket.WriteBits(GuildName.length(), 7);
    _worldPacket.FlushBits();

    _worldPacket << InviterVirtualRealmAddress;
    _worldPacket << GuildVirtualRealmAddress;
    _worldPacket << GuildGUID;
    _worldPacket << OldGuildVirtualRealmAddress;
    _worldPacket << OldGuildGUID;
    _worldPacket << EmblemStyle;
    _worldPacket << EmblemColor;
    _worldPacket << BorderStyle;
    _worldPacket << BorderColor;
    _worldPacket << Background;
    _worldPacket << Level;
    
    _worldPacket.WriteString(InviterName);
    _worldPacket.WriteString(OldGuildName);
    _worldPacket.WriteString(GuildName);
    
    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRosterProfessionData const& rosterProfessionData)
{
    data << rosterProfessionData.DbID;
    data << rosterProfessionData.Rank;
    data << rosterProfessionData.Step;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRosterMemberData const& rosterMemberData)
{
    data << rosterMemberData.Guid;
    data << rosterMemberData.RankID;
    data << rosterMemberData.AreaID;
    data << rosterMemberData.PersonalAchievementPoints;
    data << rosterMemberData.GuildReputation;
    data << rosterMemberData.LastSave;

    for (uint8 i = 0; i < 2; i++)
        data << rosterMemberData.Profession[i];

    data << rosterMemberData.VirtualRealmAddress;
    data << rosterMemberData.Status;
    data << rosterMemberData.Level;
    data << rosterMemberData.ClassID;
    data << rosterMemberData.Gender;

    data.WriteBits(rosterMemberData.Name.length(), 6);
    data.WriteBits(rosterMemberData.Note.length(), 8);
    data.WriteBits(rosterMemberData.OfficerNote.length(), 8);
    data.WriteBit(rosterMemberData.Authenticated);
    data.WriteBit(rosterMemberData.SorEligible);
    data.FlushBits();

    data.WriteString(rosterMemberData.Name);
    data.WriteString(rosterMemberData.Note);
    data.WriteString(rosterMemberData.OfficerNote);

    return data;
}

WorldPacket const* WorldPackets::Guild::GuildEventPresenceChange::Write()
{
    _worldPacket << Guid;
    _worldPacket << VirtualRealmAddress;

    _worldPacket.WriteBits(Name.length(), 6);
    _worldPacket.WriteBit(LoggedOn);
    _worldPacket.WriteBit(Mobile);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventMotd::Write()
{
    _worldPacket.WriteBits(MotdText.length(), 10);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(MotdText);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventPlayerJoined::Write()
{
    _worldPacket << Guid;
    _worldPacket << VirtualRealmAddress;
    
    _worldPacket.WriteBits(Name.length(), 6);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventRankChanged::Write()
{
    _worldPacket << RankID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventBankMoneyChanged::Write()
{
    _worldPacket << Money;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventLogQueryResults::Write()
{
    _worldPacket << uint32(Entry.size());

    for (GuildEventEntry const& entry : Entry)
    {
        _worldPacket << entry.PlayerGUID;
        _worldPacket << entry.OtherGUID;
        _worldPacket << entry.RankID;
        _worldPacket << entry.TransactionType;
        _worldPacket << entry.TransactionDate;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventPlayerLeft::Write()
{
    _worldPacket.WriteBit(Removed);
    _worldPacket.WriteBits(LeaverName.length(), 6);
    _worldPacket.FlushBits();

    if (Removed)
    {
        _worldPacket.WriteBits(RemoverName.length(), 6);
        _worldPacket.FlushBits();
        
        _worldPacket << RemoverGUID;
        _worldPacket << RemoverVirtualRealmAddress;
        _worldPacket.WriteString(RemoverName);

    }

    _worldPacket << LeaverGUID;
    _worldPacket << LeaverVirtualRealmAddress;
    _worldPacket.WriteString(LeaverName);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildPermissionsQueryResults::Write()
{
    _worldPacket << RankID;
    _worldPacket << WithdrawGoldLimit;
    _worldPacket << Flags;
    _worldPacket << NumTabs;
    _worldPacket << uint32(Tab.size());

    for (GuildRankTabPermissions const& tab : Tab)
    {
        _worldPacket << tab.Flags;
        _worldPacket << tab.WithdrawItemLimit;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventNewLeader::Write()
{
    _worldPacket.WriteBit(SelfPromoted);
    _worldPacket.WriteBits(NewLeaderName.length(), 6);
    _worldPacket.WriteBits(OldLeaderName.length(), 6);
    _worldPacket.FlushBits();

    _worldPacket << OldLeaderGUID;
    _worldPacket << OldLeaderVirtualRealmAddress;
    _worldPacket << NewLeaderGUID;
    _worldPacket << NewLeaderVirtualRealmAddress;

    _worldPacket.WriteString(NewLeaderName);
    _worldPacket.WriteString(OldLeaderName);
    
    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRankData const& rankData)
{
    data << rankData.RankID;
    data << rankData.RankOrder;
    data << rankData.Flags;
    data << rankData.WithdrawGoldLimit;

    for (uint8 i = 0; i < GUILD_BANK_MAX_TABS; i++)
    {
        data << rankData.TabFlags[i];
        data << rankData.TabWithdrawItemLimit[i];
    }

    data.WriteBits(rankData.RankName.length(), 7);
    data.FlushBits();

    data.WriteString(rankData.RankName);

    return data;
}

void WorldPackets::Guild::GuildAddRank::Read()
{
    _worldPacket.WriteBits(Name.length(), 7);
    _worldPacket.FlushBits();

    _worldPacket >> RankOrder;
    _worldPacket.WriteString(Name);
}

void WorldPackets::Guild::GuildAssignMemberRank::Read()
{
    _worldPacket >> Member;
    _worldPacket >> RankOrder;
}

void WorldPackets::Guild::GuildDeleteRank::Read()
{
    _worldPacket >> RankOrder;
}

void WorldPackets::Guild::GuildGetRanks::Read()
{
    _worldPacket >> GuildGUID;
}

WorldPacket const* WorldPackets::Guild::GuildRanks::Write()
{
    _worldPacket << uint32(Ranks.size());

    for (GuildRankData const& rank : Ranks)
        _worldPacket << rank;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildSendRankChange::Write()
{
    _worldPacket << Officer;
    _worldPacket << Other;
    _worldPacket << RankID;

    _worldPacket.WriteBit(Promote);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Guild::GuildUpdateInfoText::Read()
{
    uint32 textLen = _worldPacket.ReadBits(11);
    InfoText = _worldPacket.ReadString(textLen);
}

void WorldPackets::Guild::GuildDemoteMember::Read()
{
    _worldPacket >> Demotee;
}

void WorldPackets::Guild::GuildPromoteMember::Read()
{
    _worldPacket >> Promotee;
}

void WorldPackets::Guild::GuildChangeNameRequest::Read()
{
    uint32 nameLen = _worldPacket.ReadBits(7);
    NewName = _worldPacket.ReadString(nameLen);
}

WorldPacket const* WorldPackets::Guild::GuildFlaggedForRename::Write()
{
    _worldPacket.WriteBit(FlagSet);

    return &_worldPacket;
}

void WorldPackets::Guild::RequestGuildPartyState::Read()
{
    _worldPacket >> GuildGUID;
}

WorldPacket const* WorldPackets::Guild::GuildPartyState::Write()
{
    _worldPacket.WriteBit(InGuildParty);
    _worldPacket.FlushBits();

    _worldPacket << NumMembers;
    _worldPacket << NumRequired;
    _worldPacket << GuildXPEarnedMult;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRewardItem const& rewardItem)
{
    data << rewardItem.ItemID;
    data << uint32(rewardItem.AchievementsRequired.size());
    data << rewardItem.RaceMask;
    data << rewardItem.MinGuildLevel;
    data << rewardItem.MinGuildRep;
    data << rewardItem.Cost;

    for (uint8 i = 0; i < rewardItem.AchievementsRequired.size(); i++)
        data << rewardItem.AchievementsRequired[i];

    return data;
}

void WorldPackets::Guild::RequestGuildRewardsList::Read()
{
    _worldPacket >> CurrentVersion;
}

WorldPacket const* WorldPackets::Guild::GuildRewardList::Write()
{
    _worldPacket << Version;
    _worldPacket << uint32(RewardItems.size());

    for (GuildRewardItem const& item : RewardItems)
        _worldPacket << item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildBankTextQueryResult::Write()
{
    _worldPacket << Tab;
    _worldPacket.WriteBits(Text.length(), 14);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Text);

    return &_worldPacket;
}

void WorldPackets::Guild::GuildBankActivate::Read()
{
    _worldPacket >> Banker;
    FullUpdate = _worldPacket.ReadBit();
}

void WorldPackets::Guild::GuildBankBuyTab::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;
}

void WorldPackets::Guild::GuildBankDepositMoney::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> Money;
}

void WorldPackets::Guild::GuildBankQueryTab::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> Tab;
    FullUpdate = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Guild::GuildBankRemainingWithdrawMoney::Write()
{
    _worldPacket << RemainingWithdrawMoney;

    return &_worldPacket;
}

void WorldPackets::Guild::GuildBankWithdrawMoney::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> Money;
}

WorldPacket const* WorldPackets::Guild::GuildBankQueryResults::Write()
{
    _worldPacket << Money;
    _worldPacket << WithdrawalsRemaining;
    _worldPacket << Tab;
    _worldPacket << uint32(TabInfo.size());
    _worldPacket << uint32(ItemInfo.size());

    for (GuildBankTabInfo const& tab : TabInfo)
    {
        _worldPacket << tab.TabIndex;
        _worldPacket.WriteBits(tab.Name.length(), 7);
        _worldPacket.WriteBits(tab.Icon.length(), 9);
        _worldPacket.FlushBits();

        _worldPacket.WriteString(tab.Name);
        _worldPacket.WriteString(tab.Icon);
    }

    for (GuildBankItemInfo const& item : ItemInfo)
    {
        _worldPacket << item.Slot;
        _worldPacket << item.Item;
        _worldPacket << item.Count;
        _worldPacket << item.EnchantmentID;
        _worldPacket << item.Charges;
        _worldPacket << item.OnUseEnchantmentID;
        _worldPacket << uint32(item.SocketEnchant.size());

        for (GuildBankItemInfo::GuildBankSocketEnchant const& socketEnchant : item.SocketEnchant)
        {
            _worldPacket << socketEnchant.SocketIndex;
            _worldPacket << socketEnchant.SocketEnchantID;
        }

        _worldPacket.WriteBit(item.Locked);
        _worldPacket.FlushBits();
    }

    _worldPacket.WriteBit(FullUpdate);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

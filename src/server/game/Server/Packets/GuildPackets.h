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

#ifndef GuildPackets_h__
#define GuildPackets_h__

#include "Guild.h"
#include "Packet.h"
#include "ItemPackets.h"
#include "ObjectGuid.h"

namespace WorldPackets
{
    namespace Guild
    {
        class QueryGuildInfo final : public ClientPacket
        {
        public:
            QueryGuildInfo(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_QUERY, std::move(packet)) { }

            void Read() override;

            ObjectGuid PlayerGuid;
            ObjectGuid GuildGuid;
        };

        class QueryGuildInfoResponse final : public ServerPacket
        {
        public:
            struct GuildInfo
            {
                ObjectGuid GuildGUID;

                uint32 VirtualRealmAddress = 0; ///< a special identifier made from the Index, BattleGroup and Region.

                std::string GuildName;

                struct GuildInfoRank
                {
                    GuildInfoRank(uint32 id, uint32 order, std::string const& name)
                        : RankID(id), RankOrder(order), RankName(name) { }

                    uint32 RankID;
                    uint32 RankOrder;
                    std::string RankName;

                    bool operator<(GuildInfoRank const& right) const
                    {
                        return RankID < right.RankID;
                    }
                };

                std::set<GuildInfoRank> Ranks;

                uint32 EmblemStyle = 0;
                uint32 EmblemColor = 0;
                uint32 BorderStyle = 0;
                uint32 BorderColor = 0;
                uint32 BackgroundColor = 0;
            };

            QueryGuildInfoResponse();

            WorldPacket const* Write() override;

            ObjectGuid GuildGuid;
            Optional<GuildInfo> Info;
        };

        class GuildGetRoster final : public ClientPacket
        {
        public:
            GuildGetRoster(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_ROSTER, std::move(packet)) { }

            void Read() override { }
        };

        struct GuildRosterProfessionData
        {
            int32 DbID = 0;
            int32 Rank = 0;
            int32 Step = 0;
        };

        struct GuildRosterMemberData
        {
            ObjectGuid Guid;
            int64 WeeklyXP = 0;
            int64 TotalXP = 0;
            int32 RankID = 0;
            int32 AreaID = 0;
            int32 PersonalAchievementPoints = 0;
            int32 GuildReputation = 0;
            int32 GuildRepToCap = 0;
            float LastSave = 0.0f;
            std::string Name = "";
            uint32 VirtualRealmAddress = 0;
            std::string Note = "";
            std::string OfficerNote = "";
            uint8 Status = 0;
            uint8 Level = 0;
            uint8 ClassID = 0;
            uint8 Gender = 0;
            bool Authenticated = false;
            bool SorEligible = false;
            GuildRosterProfessionData Profession[2];
        };

        class GuildRoster final : public ServerPacket
        {
        public:
            GuildRoster() : ServerPacket(SMSG_GUILD_ROSTER, 4 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            std::vector<GuildRosterMemberData> MemberData;
            std::string WelcomeText = "";
            std::string InfoText = "";
            uint32 CreateDate = 0;
            int32 NumAccounts = 0;
            int32 MaxWeeklyRep = 0;
        };

        class GuildRosterUpdate final : public ServerPacket
        {
        public:
            GuildRosterUpdate() : ServerPacket(SMSG_GUILD_ROSTER_UPDATE, 4) { }

            WorldPacket const* Write() override;

            std::vector<GuildRosterMemberData> MemberData;
        };
        
        class GuildUpdateMotdText final : public ClientPacket
        {
        public:
            GuildUpdateMotdText(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_MOTD, std::move(packet)) { }

            void Read() override;

            std::string MotdText;
        };
        
        class GuildCommandResult final : public ServerPacket
        {
        public:
            GuildCommandResult() : ServerPacket(SMSG_GUILD_COMMAND_RESULT, 9) { }

            WorldPacket const* Write() override;

            std::string Name = "";
            int32 Result = 0;
            int32 Command = 0;
        };

        class AcceptGuildInvite final : public ClientPacket
        {
        public:
            AcceptGuildInvite(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_ACCEPT_INVITE, std::move(packet)) { }

            void Read() override { }
        };

        class GuildDeclineInvitation final : public ClientPacket
        {
        public:
            GuildDeclineInvitation(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_DECLINE_INVITATION, std::move(packet)) { }

            void Read() override { }
        }; 

        class GuildInviteByName final : public ClientPacket
        {
        public:
            GuildInviteByName(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_INVITE, std::move(packet)) { }

            void Read() override;

            std::string Name;
        };

        class GuildInvite final : public ServerPacket
        {
        public:
            GuildInvite() : ServerPacket(SMSG_GUILD_INVITE, 68) { }

            WorldPacket const* Write() override;

            ObjectGuid GuildGUID;
            ObjectGuid OldGuildGUID;
            int32 Level = 0;
            uint32 EmblemColor = 0;
            uint32 EmblemStyle = 0;
            uint32 BorderStyle = 0;
            uint32 BorderColor = 0;
            uint32 Background = 0;
            uint32 GuildVirtualRealmAddress = 0;
            uint32 OldGuildVirtualRealmAddress = 0;
            uint32 InviterVirtualRealmAddress = 0;
            std::string InviterName = "";
            std::string GuildName = "";
            std::string OldGuildName = "";
        };

        class GuildEventPresenceChange final : public ServerPacket
        {
        public:
            GuildEventPresenceChange() : ServerPacket(SMSG_GUILD_EVENT_PRESENCE_CHANGE, 16+4+1+1) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            uint32 VirtualRealmAddress = 0;
            std::string Name = "";
            bool Mobile = false;
            bool LoggedOn = false;
        };

        class GuildEventMotd final : public ServerPacket
        {
        public:
            GuildEventMotd() : ServerPacket(SMSG_GUILD_EVENT_MOTD, 1) { }

            WorldPacket const* Write() override;

            std::string MotdText = "";
        };

        class GuildEventPlayerJoined final : public ServerPacket
        {
        public:
            GuildEventPlayerJoined() : ServerPacket(SMSG_GUILD_EVENT_PLAYER_JOINED, 21) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            std::string Name = "";
            uint32 VirtualRealmAddress = 0;
        };

        class GuildEventRankChanged final : public ServerPacket
        {
        public:
            GuildEventRankChanged() : ServerPacket(SMSG_GUILD_EVENT_RANK_CHANGED, 4) { }

            WorldPacket const* Write() override;

            int32 RankID = 0;
        };

        class GuildEventBankMoneyChanged final : public ServerPacket
        {
        public:
            GuildEventBankMoneyChanged() : ServerPacket(SMSG_GUILD_EVENT_BANK_MONEY_CHANGED, 8) { }

            WorldPacket const* Write() override;

            uint64 Money = 0;
        };

        class GuildEventDisbanded final : public ServerPacket
        {
        public:
            GuildEventDisbanded() : ServerPacket(SMSG_GUILD_EVENT_DISBANDED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        struct GuildEventEntry
        {
            ObjectGuid PlayerGUID;
            ObjectGuid OtherGUID;
            uint32 TransactionDate = 0;
            uint8 TransactionType = 0;
            uint8 RankID = 0;
        };

        class GuildEventLogQuery final : public ClientPacket
        {
        public:
            GuildEventLogQuery(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_EVENT_LOG_QUERY, std::move(packet)) { }

            void Read() override { }
        };

        class GuildEventLogQueryResults final : public ServerPacket
        {
        public:
            GuildEventLogQueryResults() : ServerPacket(SMSG_GUILD_EVENT_LOG_QUERY_RESULT, 4) { }

            WorldPacket const* Write() override;

            std::vector<GuildEventEntry> Entry;
        };

        class GuildEventPlayerLeft final : public ServerPacket
        {
        public:
            GuildEventPlayerLeft() : ServerPacket(SMSG_GUILD_EVENT_PLAYER_LEFT, 43) { }

            WorldPacket const* Write() override;

            ObjectGuid LeaverGUID;
            std::string LeaverName = "";
            uint32 LeaverVirtualRealmAddress = 0;
            ObjectGuid RemoverGUID;
            std::string RemoverName = "";
            uint32 RemoverVirtualRealmAddress = 0;
            bool Removed = false;
        };

        class GuildEventNewLeader final : public ServerPacket
        {
        public:
            GuildEventNewLeader() : ServerPacket(SMSG_GUILD_EVENT_NEW_LEADER, 43) { }

            WorldPacket const* Write() override;

            ObjectGuid NewLeaderGUID;
            std::string NewLeaderName = "";
            uint32 NewLeaderVirtualRealmAddress = 0;
            ObjectGuid OldLeaderGUID;
            std::string OldLeaderName = "";
            uint32 OldLeaderVirtualRealmAddress = 0;
            bool SelfPromoted = false;
        };
        
        class GuildPermissionsQuery final : public ClientPacket
        {
        public:
            GuildPermissionsQuery(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_PERMISSIONS, std::move(packet)) { }

            void Read() override { }
        };

        class GuildPermissionsQueryResults final : public ServerPacket
        {
        public:
            struct GuildRankTabPermissions
            {
                int32 Flags = 0;
                int32 WithdrawItemLimit = 0;
            };

            GuildPermissionsQueryResults() : ServerPacket(SMSG_GUILD_PERMISSIONS_QUERY_RESULTS, 20) { }

            WorldPacket const* Write() override;

            int32 NumTabs = 0;
            int32 WithdrawGoldLimit = 0;
            int32 Flags = 0;
            uint32 RankID = 0;
            std::vector<GuildRankTabPermissions> Tab;
        };
        
        class GuildAddRank final : public ClientPacket
        {
        public:
            GuildAddRank(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_ADD_RANK, std::move(packet)) { }

            void Read() override;

            std::string Name;
            int32 RankOrder;
        };

        class GuildAssignMemberRank final : public ClientPacket
        {
        public:
            GuildAssignMemberRank(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_ASSIGN_MEMBER_RANK, std::move(packet)) { }

            void Read() override;

            ObjectGuid Member;
            int32 RankOrder;
        };

        class GuildDeleteRank final : public ClientPacket
        {
        public:
            GuildDeleteRank(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_DEL_RANK, std::move(packet)) { }

            void Read() override;

            int32 RankOrder;
        };

        class GuildGetRanks final : public ClientPacket
        {
        public:
            GuildGetRanks(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_GET_RANKS, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
        };

        struct GuildRankData
        {
            uint32 RankID = 0;
            uint32 RankOrder = 0;
            uint32 Flags = 0;
            uint32 WithdrawGoldLimit = 0;
            std::string RankName = "";
            uint32 TabFlags[GUILD_BANK_MAX_TABS];
            uint32 TabWithdrawItemLimit[GUILD_BANK_MAX_TABS];
        };

        class GuildRanks final : public ServerPacket
        {
        public:
            GuildRanks() : ServerPacket(SMSG_GUILD_RANK, 4) { }

            WorldPacket const* Write() override;

            std::vector<GuildRankData> Ranks;
        };

        class GuildSendRankChange final : public ServerPacket
        {
        public:
            GuildSendRankChange() : ServerPacket(SMSG_GUILD_RANKS_UPDATE, 43) { }

            WorldPacket const* Write() override;

            ObjectGuid Other;
            ObjectGuid Officer;
            bool Promote = false;
            uint32 RankID = 0;
        };

        class GuildUpdateInfoText final : public ClientPacket
        {
        public:
            GuildUpdateInfoText(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_INFO_TEXT, std::move(packet)) { }

            void Read() override;

            std::string InfoText;
        };

        class GuildMemberDailyReset final : public ServerPacket
        {
        public:
            GuildMemberDailyReset() : ServerPacket(SMSG_GUILD_MEMBER_DAILY_RESET, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class GuildDelete final : public ClientPacket
        {
        public:
            GuildDelete(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_DELETE, std::move(packet)) { }

            void Read() override { }
        };

        class GuildDemoteMember final : public ClientPacket
        {
        public:
            GuildDemoteMember(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_DEMOTE, std::move(packet)) { }

            void Read() override;

            ObjectGuid Demotee;
        };

        class GuildPromoteMember final : public ClientPacket
        {
        public:
            GuildPromoteMember(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_PROMOTE, std::move(packet)) { }

            void Read() override;

            ObjectGuid Promotee;
        };

        class GuildLeave final : public ClientPacket
        {
        public:
            GuildLeave(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_LEAVE, std::move(packet)) { }

            void Read() override { }
        };

        class RequestGuildPartyState final : public ClientPacket
        {
        public:
            RequestGuildPartyState(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_REQUEST_PARTY_STATE, std::move(packet)) { }

            void Read() override;

            ObjectGuid GuildGUID;
        };

        class GuildPartyState final : public ServerPacket
        {
        public:
            GuildPartyState() : ServerPacket(SMSG_GUILD_PARTY_STATE_RESPONSE, 15) { }

            WorldPacket const* Write() override;

            float GuildXPEarnedMult = 0.0f;
            int32 NumMembers = 0;
            int32 NumRequired = 0;
            bool InGuildParty = false;
        };

        class RequestGuildRewardsList final : public ClientPacket
        {
        public:
            RequestGuildRewardsList(WorldPacket&& packet) : ClientPacket(CMSG_QUERY_GUILD_REWARDS, std::move(packet)) { }

            void Read() override;

            uint32 CurrentVersion;
        };

        struct GuildRewardItem
        {
            uint32 ItemID = 0;
            std::vector<uint32> AchievementsRequired;
            uint32 RaceMask = 0;
            int32 MinGuildLevel = 0;
            int32 MinGuildRep = 0;
            uint64 Cost = 0;
        };

        class GuildRewardList final : public ServerPacket
        {
        public:
            GuildRewardList() : ServerPacket(SMSG_GUILD_REWARDS_LIST, 8) { }

            WorldPacket const* Write() override;

            std::vector<GuildRewardItem> RewardItems;
            uint32 Version = 0;
        };

        class GuildBankTextQueryResult : public ServerPacket
        {
        public:
            GuildBankTextQueryResult() : ServerPacket(SMSG_GUILD_BANK_QUERY_TEXT_RESULT, 5) { }

            WorldPacket const* Write() override;

            int32 Tab = 0;
            std::string Text;
        };
        
        class GuildBankBuyTab final : public ClientPacket
        {
        public:
            GuildBankBuyTab(WorldPacket&& packet) : ClientPacket(CMSG_GUILD_BANK_BUY_TAB, std::move(packet)) { }

            void Read() override;

            ObjectGuid Banker;
            uint8 BankTab = 0;
        };

        class GuildBankQueryResults final : public ServerPacket
        {
        public:
            struct GuildBankItemInfo
            {
                struct GuildBankSocketEnchant
                {
                    int32 SocketIndex = 0;
                    int32 SocketEnchantID = 0;
                };

                WorldPackets::Item::ItemInstance Item;
                int32 Slot = 0;
                int32 Count = 0;
                int32 EnchantmentID = 0;
                int32 Charges = 0;
                int32 OnUseEnchantmentID = 0;
                int32 Flags = 0;
                bool Locked = false;
                std::vector<GuildBankSocketEnchant> SocketEnchant;
            };

            struct GuildBankTabInfo
            {
                int32 TabIndex = 0;
                std::string Name = "";
                std::string Icon = "";
            };

            GuildBankQueryResults() : ServerPacket(SMSG_GUILD_BANK_LIST, 25) { }

            WorldPacket const* Write() override;

            std::vector<GuildBankItemInfo> ItemInfo;
            std::vector<GuildBankTabInfo> TabInfo;
            int32 WithdrawalsRemaining = 0;
            int32 Tab = 0;
            uint64 Money = 0;
            bool FullUpdate = false;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRosterProfessionData const& rosterProfessionData);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRosterMemberData const& rosterMemberData);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRankData const& rankData);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRewardItem const& rewardItem);

#endif // GuildPackets_h__

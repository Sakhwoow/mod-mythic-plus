/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "mythic_plus.h"

class mythic_plus_keystone : public ItemScript
{
public:
    mythic_plus_keystone() : ItemScript("mythic_plus_keystone") {}

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        Map* map = player->GetMap();
        if (sMythicPlus->CanMapBeMythicPlus(map))
        {
            // only group's leader can use the keystone
            if (!MythicPlus::Utils::IsGroupLeader(player))
                MythicPlus::BroadcastToPlayer(player, "Только лидер группы может использовать ключ.");
            else
            {
                // now check if the group's leader actually has a set m+ level
                if (sMythicPlus->GetCurrentMythicPlusLevel(player) == 0)
                    MythicPlus::BroadcastToPlayer(player, "У вас не выбрана сложность Мифик+");
                else
                {
                    MythicPlus::MapData* mapData = sMythicPlus->GetMapData(map);
                    if (mapData->mythicPlusStartTimer > 0)
                        MythicPlus::BroadcastToPlayer(player, "Подземелье со сложностью Мифик+ уже запущено или завершено.");
                    else if (mapData->keystoneTimer > 0)
                        MythicPlus::BroadcastToPlayer(player, "Ключ уже был использован и ожидал запуска Мифик+ сложности...");
                    else
                    {
                        // lets check if a dungeon save was performed, in which case it must be saved as M+
                        const MythicPlus::MythicPlusDungeonInfo* dsave = sMythicPlus->GetSavedDungeonInfo(map->GetInstanceId());
                        if (dsave != nullptr && !dsave->isMythic)
                            MythicPlus::BroadcastToPlayer(player, "Это подземелье не было отмечено сложностью Мифик+, ключ больше нельзя использовать.");
                        else
                        {
                            if (player->IsInCombat())
                                MythicPlus::BroadcastToPlayer(player, "Нельзя использовать ключ в бою.");
                            else
                            {
                                // every player in the group must be online and at max level
                                if (!sMythicPlus->CheckGroupLevelForKeystone(player))
                                    MythicPlus::BroadcastToPlayer(player, "Все игроки в группе, должны находиться онлайн или иметь подходящий уровень.");
                                else
                                {
                                    mapData->keystoneTimer = MythicPlus::Utils::GameTimeCount();
                                    mapData->keystoneLevel = sMythicPlus->GetCurrentMythicPlusLevel(player);
                                    std::ostringstream oss;
                                    oss << "Mythic Plus will start in ";
                                    oss << secsToTimeString(MythicPlus::KEYSTONE_START_TIMER / 1000);
                                    oss << ". Mythic level set: ";
                                    oss << Acore::ToString(mapData->keystoneLevel);
                                    MythicPlus::AnnounceToGroup(player, oss.str());

                                    sMythicPlus->RemoveKeystone(player);
                                }
                            }
                        }
                    }
                }
            }
        }
        else
            MythicPlus::BroadcastToPlayer(player, "Чтобы использовать ключ необходимо находятся в подземелье с выбранной сложностью Мифик+.");
        return true;
    }
};

void AddSC_mythic_plus_keystone()
{
    new mythic_plus_keystone();
}

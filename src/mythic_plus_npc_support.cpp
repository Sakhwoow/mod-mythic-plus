/*
 * Credits: silviu20092
 */

#include "Player.h"
#include "Creature.h"
#include "ScriptedGossip.h"
#include "StringConvert.h"
#include "Group.h"
#include "mythic_plus.h"
#include "mythic_affix.h"
#include "mythic_plus_npc_support.h"

void MythicPlusNpcSupport::AddMainMenu(Player* player, Creature* creature)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.backMenu = false;
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_NPC_MENU;

    if (!sMythicPlus->IsEnabled())
    {
        Identifier* disabledIdnt = new Identifier();
        disabledIdnt->id = 0;
        disabledIdnt->uiName = MythicPlus::Utils::RedColored("!!! SYSTEM IS NOT ACTIVE !!!");
        pagedData.data.push_back(disabledIdnt);
    }

    Identifier* i1 = new Identifier();
    i1->id = 1;
    i1->uiName = "Выберете сложность";
    i1->optionIcon = GOSSIP_ICON_BATTLE;
    pagedData.data.push_back(i1);

    uint32 setLevel = sMythicPlus->GetCurrentMythicPlusLevel(player);
    if (setLevel > 0)
    {
        Identifier* resetIdnt = new Identifier();
        resetIdnt->id = 2;
        resetIdnt->uiName = "Сбросить сложность [ТЕКУЩИЙ: " + Acore::ToString(setLevel) + "]";
        resetIdnt->optionIcon = GOSSIP_ICON_BATTLE;
        pagedData.data.push_back(resetIdnt);
    }
    else
    {
        Identifier* nothingIdnt = new Identifier();
        nothingIdnt->id = 3;
        nothingIdnt->uiName = MythicPlus::Utils::Colored("Ни какой сложности не установлено", "b50505");
        nothingIdnt->optionIcon = GOSSIP_ICON_CHAT;
        pagedData.data.push_back(nothingIdnt);
    }

    if (player->GetGroup() != nullptr)
    {
        ObjectGuid leaderGuid = player->GetGroup()->GetLeaderGUID();
        uint32 leaderLevel = sMythicPlus->GetCurrentMythicPlusLevelForGUID(leaderGuid.GetCounter());
        Player* leader = ObjectAccessor::FindConnectedPlayer(leaderGuid);
        Identifier* dungeonLevelIdnt = new Identifier();
        dungeonLevelIdnt->id = 4;
        dungeonLevelIdnt->optionIcon = GOSSIP_ICON_CHAT;
        std::ostringstream oss;
        oss << "Текущая сложность  (зависит от лидера): ";
        if (leaderLevel == 0)

            oss << MythicPlus::Utils::Colored("НЕ УСТАНОВЛЕНО (0)", "b50505");
        else
            oss << leaderLevel;
        if (!leader)
            oss << " [ЛИДЕР НЕ В ИГРЕ]";
        dungeonLevelIdnt->uiName = oss.str();
        pagedData.data.push_back(dungeonLevelIdnt);
    }

    Identifier* mPlusListIdnt = new Identifier();
    mPlusListIdnt->id = 5;
    mPlusListIdnt->uiName = "Список подземелий ";
    pagedData.data.push_back(mPlusListIdnt);

    Identifier* standingsRefreshIdnt = new Identifier();
    standingsRefreshIdnt->id = 6;
    std::ostringstream oss;
    oss << "Обновление таблицы: ";
    oss << MythicPlus::Utils::Colored(secsToTimeString((MythicPlus::MYTHIC_SNAPSHOTS_TIMER_FREQ - sMythicPlus->GetMythicSnapshotsTimer()) / 1000), "b50505");
    standingsRefreshIdnt->uiName = oss.str();
    pagedData.data.push_back(standingsRefreshIdnt);

    Identifier* standings = new Identifier();
    standings->id = 7;
    standings->uiName = "Текущая таблица ->";
    pagedData.data.push_back(standings);

    Identifier* keystoneIdnt = new Identifier();
    keystoneIdnt->id = 8;
    std::ostringstream koss;
    koss << MythicPlus::Utils::Colored("Получить ключ ", "700c63");
    if (sMythicPlus->GetKeystoneBuyTimer() > 0)
    {
        uint32 playerKeystoneBuyTimer = sMythicPlus->GetKeystoneBuyTimer(player);
        std::string available = MythicPlus::Utils::GreenColored(" [ДОСТУПНО]");
        if (playerKeystoneBuyTimer > 0)
        {
            uint64 now = MythicPlus::Utils::GameTimeCount();
            uint64 diff = now - playerKeystoneBuyTimer;
            if (diff < sMythicPlus->GetKeystoneBuyTimer() * 60)
                available = MythicPlus::Utils::RedColored("- Станет доступен через" + secsToTimeString(sMythicPlus->GetKeystoneBuyTimer() * 60 - diff));
        }
        koss << available;
    }
    keystoneIdnt->uiName = koss.str();
    keystoneIdnt->optionIcon = GOSSIP_ICON_MONEY_BAG;
    pagedData.data.push_back(keystoneIdnt);

    Identifier* randomMythicIdnt = new Identifier();
    randomMythicIdnt->id = 9;
    randomMythicIdnt->uiName = "Список возможных случайных усилений ->";
    randomMythicIdnt->optionIcon = GOSSIP_ICON_BATTLE;
    pagedData.data.push_back(randomMythicIdnt);

    Identifier* bye = new Identifier();
    bye->id = 10;
    bye->uiName = "Не обращайте внимания...";
    pagedData.data.push_back(bye);

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusLevels(Player* player)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVELS;

    const MythicLevelContainer& mythicLevels = sMythicPlus->GetAllMythicLevels();
    for (const auto& mlevel : mythicLevels)
    {
        Identifier* idnt = new Identifier();
        idnt->id = mlevel.level;
        idnt->optionIcon = GOSSIP_ICON_BATTLE;
        std::ostringstream oss;
        oss << "Сложность - " << mlevel.level;
        oss << " (" << mlevel.affixes.size() << " усиление)";
        if (mlevel.randomAffixCount > 0)
            oss << " (" << mlevel.randomAffixCount << " случайное усиление)";
        oss << " ->";
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusLevelInfo(Player* player, uint32 mythicLevel)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVEL_INFO;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mythicLevel = mythicLevel;

    uint32 id = 0;

    Identifier* idnt = new Identifier();
    idnt->id = ++id;
    idnt->optionIcon = GOSSIP_ICON_BATTLE;
    idnt->uiName = MythicPlus::Utils::Colored("Нажмите, чтобы выбрать сложность " + Acore::ToString(mythicLevel), "0a4a0e");
    pagedData.data.push_back(idnt);

    const MythicLevel* level = sMythicPlus->GetMythicLevel(mythicLevel);
    ASSERT(level);

    Identifier* timerIdnt = new Identifier();
    timerIdnt->id = ++id;
    timerIdnt->uiName = "Ограничение по времени: " + secsToTimeString(level->timeLimit);
    pagedData.data.push_back(timerIdnt);

    for (int i = 0; i < level->affixes.size(); i++)
    {
        const MythicAffix* affix = level->affixes.at(i);

        Identifier* affixIdnt = new Identifier();
        affixIdnt->id = ++id;
        std::ostringstream oss;
        oss << "Усиление ";
        oss << i + 1 << ": ";
        oss << affix->ToString();
        if (affix->IsRandom())
            oss << MythicPlus::Utils::Colored(" (Случайно)", "1a0966");
        affixIdnt->uiName = oss.str();
        pagedData.data.push_back(affixIdnt);
    }

    Identifier* rewardsIdnt = new Identifier();
    rewardsIdnt->id = ++id;
    rewardsIdnt->optionIcon = GOSSIP_ICON_MONEY_BAG;
    rewardsIdnt->uiName = MythicPlus::Utils::Colored("Награда за прохождение", "0d852d");
    pagedData.data.push_back(rewardsIdnt);

    const MythicReward& reward = level->reward;
    if (reward.money)
    {
        Identifier* moneyIdnt = new Identifier();
        moneyIdnt->id = ++id;
        moneyIdnt->optionIcon = GOSSIP_ICON_MONEY_BAG;
        moneyIdnt->uiName = "Золото: " + MythicPlus::Utils::CopperToMoneyStr(reward.money, false);
        pagedData.data.push_back(moneyIdnt);
    }

    if (!reward.tokens.empty())
    {
        for (const auto& token : reward.tokens)
        {
            Identifier* tokenIdnt = new Identifier();
            tokenIdnt->id = ++id;
            tokenIdnt->optionIcon = GOSSIP_ICON_VENDOR;
            std::ostringstream oss;
            oss << MythicPlus::Utils::ItemLinkForUI(token.first, player);
            oss << " - x" << token.second;
            tokenIdnt->uiName = oss.str();
            pagedData.data.push_back(tokenIdnt);
        }
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusDungeonList(Player* player)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST;

    const std::unordered_map<uint32, MythicPlus::MythicPlusCapableDungeon>& dungeons = sMythicPlus->GetAllMythicPlusDungeons();
    LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
    uint32 id = 0;
    for (const auto& dpair : dungeons)
    {
        uint32 mapEntry = dpair.first;
        Difficulty diff = dpair.second.minDifficulty;

        MapEntry const* map = sMapStore.LookupEntry(mapEntry);
        ASSERT(map);

        Identifier* idnt = new Identifier();
        idnt->id = ++id;
        std::ostringstream oss;
        oss << map->name[locale];
        if (diff == DUNGEON_DIFFICULTY_NORMAL)
        {
            if (MythicPlus::Utils::CanBeHeroic(mapEntry))
                oss << "- (Об/Гер)";
            else
                oss << "- (Об)";
        }
        else
            oss << "ТОЛЬКО ГЕРОЙЧЕСКИЙ]";
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusDungeonListForSnapshots(Player* player, uint32 snapMythicLevel)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel = snapMythicLevel;

    Identifier* mlevelIdnt = new Identifier();
    mlevelIdnt->id = 1;
    std::ostringstream oss;
    oss << "Выбранная сложность: ";
    if (snapMythicLevel == 0)
        oss << "ВСЕ";
    else
        oss << snapMythicLevel;
    mlevelIdnt->uiName = oss.str();
    mlevelIdnt->optionIcon = GOSSIP_ICON_CHAT;
    pagedData.data.push_back(mlevelIdnt);

    const std::unordered_map<uint32, MythicPlus::MythicPlusCapableDungeon>& dungeons = sMythicPlus->GetAllMythicPlusDungeons();
    LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
    for (const auto& dpair : dungeons)
    {
        uint32 mapEntry = dpair.first;

        MapEntry const* map = sMapStore.LookupEntry(mapEntry);
        ASSERT(map);

        const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>> snapshots = sMythicPlus->GetMapSnapshot(mapEntry, snapMythicLevel);

        Identifier* idnt = new Identifier();
        idnt->id = mapEntry;
        std::ostringstream oss;
        oss << map->name[locale];
        oss << "- (Прохождений: ";
        if (snapshots.empty())
            oss << MythicPlus::Utils::Colored("Нет боев", "b50505");
        else
            oss << snapshots.size();
        oss << ")";
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusSnapshotAllRuns(Player* player, uint32 mapEntry)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT_RUNS;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mapEntry = mapEntry;

    MapEntry const* map = sMapStore.LookupEntry(mapEntry);
    ASSERT(map);

    LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();

    Identifier* mapIdnt = new Identifier();
    mapIdnt->id = 1;
    std::ostringstream oss;
    oss << "Лучшее время ";
    oss << map->name[locale];
    mapIdnt->uiName = oss.str();
    pagedData.data.push_back(mapIdnt);

    uint32 snapMythicLevel = pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel;

    const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>> snapshots = sMythicPlus->GetMapSnapshot(mapEntry, snapMythicLevel);
    if (!snapshots.empty())
    {
        uint32 id = 1;
        for (const auto& s : snapshots)
        {
            const MythicPlus::MythicPlusDungeonSnapshot& snap = s.second.at(0);
            uint32 internalId = snap.internalId;

            Identifier* idnt = new Identifier();
            idnt->id = internalId + 10;
            std::ostringstream oss;
            oss << id++ << ". ";
            if (snap.totalTime > 0)
            {
                oss << secsToTimeString(snap.totalTime);
                oss << " = Время ";
                oss << secsToTimeString(snap.timelimit);
                oss << "";
                if (snap.rewarded)
                    oss << MythicPlus::Utils::GreenColored(" Успешно");
                else
                    oss << MythicPlus::Utils::RedColored(" Провалено");
            }
            else
            {
                oss << MythicPlus::Utils::RedColored("Не закончено");
                oss << " - Время: ";
                oss << secsToTimeString(snap.timelimit);
                oss << "";
            }
            oss << " (M+";
            oss << snap.mythicLevel;
            oss << ")";
            if (snap.difficulty == DUNGEON_DIFFICULTY_NORMAL)
                oss << " Об";
            else
                oss << MythicPlus::Utils::Colored(" Гер", "9e1849");
            idnt->uiName = oss.str();
            pagedData.data.push_back(idnt);
        }
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusAllLevels(Player* player)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_ALL_LEVELS;

    Identifier* allIdnt = new Identifier();
    allIdnt->id = 0;
    allIdnt->uiName = "Все уровни";
    pagedData.data.push_back(allIdnt);

    const MythicLevelContainer& mythicLevels = sMythicPlus->GetAllMythicLevels();
    for (const auto& mlevel : mythicLevels)
    {
        Identifier* idnt = new Identifier();
        idnt->id = mlevel.level;
        idnt->optionIcon = GOSSIP_ICON_BATTLE;
        std::ostringstream oss;
        oss << "Сложность " << mlevel.level << " - >";
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusDungeonSnapshotDetails(Player* player, uint32 internalId)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_SNAPSHOT_DETAILS;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->internalId = internalId;

    uint32 mapEntry = pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mapEntry;
    uint32 snapMythicLevel = pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel;
    const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>> snapshots = sMythicPlus->GetMapSnapshot(mapEntry, snapMythicLevel);

    std::vector<MythicPlus::MythicPlusDungeonSnapshot> chosenSnaps;
    for (const auto& s : snapshots)
    {
        if (s.second.at(0).internalId == internalId)
        {
            chosenSnaps = s.second;
            break;
        }
    }
    if (chosenSnaps.empty())
        return;

    std::sort(chosenSnaps.begin(), chosenSnaps.end(), [](const MythicPlus::MythicPlusDungeonSnapshot& a, const MythicPlus::MythicPlusDungeonSnapshot& b) {
        return a.snapTime < b.snapTime;
    });

    MapEntry const* map = sMapStore.LookupEntry(mapEntry);
    ASSERT(map);

    LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();

    const MythicPlus::MythicPlusDungeonSnapshot* csnap = &chosenSnaps.at(0);

    Identifier* idnt = new Identifier();
idnt->id = 1;
std::ostringstream oss;
oss << "Подземелье: ";
oss << map->name[locale];
oss << " Сложность М+";
oss << csnap->mythicLevel;
idnt->uiName = oss.str();
pagedData.data.push_back(idnt);

oss.str("");
oss.clear();

Identifier* startTimeIdnt = new Identifier();
startTimeIdnt->id = 2;
oss << "Начато: ";
oss << MythicPlus::Utils::DateFromSeconds(csnap->startTime);
startTimeIdnt->uiName = oss.str();
pagedData.data.push_back(startTimeIdnt);

oss.str("");
oss.clear();

Identifier* endTimeIdnt = new Identifier();
endTimeIdnt->id = 3;
if (csnap->totalTime > 0)
{
    oss << "Завершено: ";
    oss << MythicPlus::Utils::DateFromSeconds(csnap->endTime);
    oss << "\n";  // новая строка
    oss << "Длительность: ";
    oss << secsToTimeString(csnap->totalTime);
}
else
    oss << MythicPlus::Utils::RedColored("Не удалось пройти");
endTimeIdnt->uiName = oss.str();
pagedData.data.push_back(endTimeIdnt);

oss.str("");
oss.clear();

Identifier* deathsIdnt = new Identifier();
deathsIdnt->id = 4;
if (csnap->totalDeaths > 0)
{
    oss << "Смертей: ";
    oss << MythicPlus::Utils::RedColored(Acore::ToString(csnap->totalDeaths));
    oss << "\n";  // новая строка
    oss << "Штрафное время: ";
    oss << secsToTimeString(csnap->penaltyOnDeath * csnap->totalDeaths);
}
else
    oss << MythicPlus::Utils::GreenColored("Без смертей");
deathsIdnt->uiName = oss.str();
pagedData.data.push_back(deathsIdnt);

uint32 id = 4;
for (const auto& s : chosenSnaps)
{
    oss.str("");
    oss.clear();

    oss << MythicPlus::Utils::Colored(MythicPlus::Utils::GetCreatureNameByEntry(player, s.entry), "102163");
    oss << " - повержен ";
    oss << MythicPlus::Utils::DateFromSeconds(s.snapTime);
    oss << "\n";  // новая строка
    oss << "Время сражения: ";
    oss << secsToTimeString(s.combatTime);
    oss << "\n";  // новая строка
    oss << "Игроки: ";
    oss << MythicPlus::Utils::Colored(s.players, "6e1849");

    if (s.randomAffixCount > 0)
        oss << "\nСлучайное усиление: " << s.randomAffixCount;

    Identifier* idnt = new Identifier();
    idnt->id = ++id;
    idnt->uiName = oss.str();
    pagedData.data.push_back(idnt);
}

if (csnap->totalTime > 0)
{
    Identifier* rewardIdnt = new Identifier();
    rewardIdnt->id = ++id;
    if (csnap->rewarded)
        rewardIdnt->uiName = MythicPlus::Utils::GreenColored("ВРЕМЯ НЕ ВЫШЛО - НАГРАДЫ ПОЛУЧЕНЫ");
    else
        rewardIdnt->uiName = MythicPlus::Utils::RedColored("НАГРАДЫ НЕ ПОЛУЧЕНЫ - ПРЕВЫШЕН ЛИМИТ ВРЕМЕНИ");
    pagedData.data.push_back(rewardIdnt);
}

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddRandomAfixes(Player* player)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_RANDOM_AFFIXES;

    uint32 id = 1;

    Identifier* infoIdnt = new Identifier();
    infoIdnt->id = id++;
    infoIdnt->uiName = "На некоторых сложностях может быть задано определенное количество случайных усилений, которые будут выбраны из списка. Случайные усиление меняются при каждом перезапуске сервера.";
    infoIdnt->optionIcon = GOSSIP_ICON_CHAT;
    pagedData.data.push_back(infoIdnt);

    const MythicLevelContainer& mythicLevels = sMythicPlus->GetAllMythicLevels();
    for (const auto& mlevel : mythicLevels)
    {
        if (mlevel.randomAffixCount > 0)
        {
            Identifier* idnt = new Identifier();
            idnt->id = 100 + mlevel.level;
            std::ostringstream oss;
            oss << "Сложность ";
            oss << mlevel.level;
            oss << " из ";
            oss << mlevel.randomAffixCount << " списка случайных усилений->";
            idnt->uiName = oss.str();
            pagedData.data.push_back(idnt);
        }
    }

    Identifier* affixesInfoIdnt = new Identifier();
    affixesInfoIdnt->id = 1000 + (id++);
    affixesInfoIdnt->uiName = "Список случайных усилений:";
    affixesInfoIdnt->optionIcon = GOSSIP_ICON_CHAT;
    pagedData.data.push_back(affixesInfoIdnt);

    for (uint32 i = 0; i < MythicAffix::RANDOM_AFFIX_MAX_COUNT; i++)
    {
        MythicAffix* affix = MythicAffix::AffixFactory((MythicAffixType)MythicAffix::RandomAffixes[i]);
        ASSERT(affix && affix->IsRandom());

        Identifier* affixIdnt = new Identifier();
        affixIdnt->id = 1000 + (id++);
        std::ostringstream aoss;
        aoss << (i + 1) << ". ";
        aoss << MythicPlus::Utils::Colored(affix->ToString(), "1a0966");
        affixIdnt->uiName = aoss.str();
        pagedData.data.push_back(affixIdnt);

        delete affix;
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddRandomAffixesForLevel(Player* player, uint32 level)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_RANDOM_AFFIXES_FOR_LEVEL;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->randomMythicLevel = level;

    uint32 id = 1;
    Identifier* levelIdnt = new Identifier();
    levelIdnt->id = id++;
    levelIdnt->uiName = "Случайное сгенерированное усиление " + Acore::ToString(level);
    levelIdnt->optionIcon = GOSSIP_ICON_CHAT;
    pagedData.data.push_back(levelIdnt);

    const MythicLevel* mythicLevel = sMythicPlus->GetMythicLevel(level);
    ASSERT(mythicLevel);

    uint32 affixIndex = 1;
    for (const auto* a : mythicLevel->affixes)
    {
        if (a->IsRandom())
        {
            Identifier* affixIdnt = new Identifier();
            affixIdnt->id = id++;
            std::ostringstream aoss;
            aoss << affixIndex++ << ". ";
            aoss << MythicPlus::Utils::Colored(a->ToString(), "1a0966");
            affixIdnt->uiName = aoss.str();
            pagedData.data.push_back(affixIdnt);
        }
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

bool MythicPlusNpcSupport::TakePagedDataAction(Player* player, Creature* creature, uint32 action)
{
    PagedData& pagedData = GetPagedData(player);
    if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_NPC_MENU)
    {
        if (action == 1)
        {
            AddMythicPlusLevels(player);
            return AddPagedData(player, creature, 0);
        }
        else if (action == 2)
        {
            if (sMythicPlus->SetCurrentMythicPlusLevel(player, 0))
            {
                MythicPlus::BroadcastToPlayer(player, "Ваша сложность была сброшена!");
                MythicPlus::Utils::VisualFeedback(player);
            }
            else
                MythicPlus::BroadcastToPlayer(player, "Вы не можете сбросить сложность, находясь в группе.");

            CloseGossipMenuFor(player);
            return true;
        }
        else if (action == 3 || action == 4 || action == 6 || action == 0)
            return OnGossipHello(player, creature);
        else if (action == 5)
        {
            AddMythicPlusDungeonList(player);
            return AddPagedData(player, creature, 0);
        }
        else if (action == 7)
        {
            AddMythicPlusAllLevels(player);
            return AddPagedData(player, creature, 0);
        }
        else if (action == 8)
        {
            if (sMythicPlus->GiveKeystone(player))
            {
                CloseGossipMenuFor(player);
                return true;
            }
            else
                return OnGossipHello(player, creature);
        }
        else if (action == 9)
        {
            AddRandomAfixes(player);
            return AddPagedData(player, creature, 0);
        }
        else if (action == 10)
        {
            CloseGossipMenuFor(player);
            return true;
        }
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVELS)
    {
        AddMythicPlusLevelInfo(player, action);
        return AddPagedData(player, creature, 0);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVEL_INFO)
    {
        uint32 chosenMythicLevel = pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mythicLevel;
        if (action == 1)
        {
            if (sMythicPlus->SetCurrentMythicPlusLevel(player, chosenMythicLevel))
            {
                MythicPlus::BroadcastToPlayer(player, "Ваша сложность была установлена на " + Acore::ToString(chosenMythicLevel));
                MythicPlus::Utils::VisualFeedback(player);
            }
            else
                MythicPlus::BroadcastToPlayer(player, "Вы не можете установить сложность, находясь в группе.");

            CloseGossipMenuFor(player);
            return true;
        }
        else
        {
            AddMythicPlusLevelInfo(player, chosenMythicLevel);
            return AddPagedData(player, creature, pagedData.currentPage);
        }
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST)
    {
        AddMythicPlusDungeonList(player);
        return AddPagedData(player, creature, pagedData.currentPage);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT)
    {
        if (action == 1)
        {
            AddMythicPlusDungeonListForSnapshots(player, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel);
            return AddPagedData(player, creature, pagedData.currentPage);
        }
        AddMythicPlusSnapshotAllRuns(player, action);
        return AddPagedData(player, creature, 0);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_ALL_LEVELS)
    {
        AddMythicPlusDungeonListForSnapshots(player, action);
        return AddPagedData(player, creature, 0);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT_RUNS)
    {
        if (action == 1)
        {
            AddMythicPlusSnapshotAllRuns(player, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mapEntry);
            return AddPagedData(player, creature, pagedData.currentPage);
        }
        else
        {
            AddMythicPlusDungeonSnapshotDetails(player, action - 10);
            return AddPagedData(player, creature, 0);
        }
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_SNAPSHOT_DETAILS)
    {
        AddMythicPlusDungeonSnapshotDetails(player, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->internalId);
        return AddPagedData(player, creature, pagedData.currentPage);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_RANDOM_AFFIXES)
    {
        if (action >= 100 && action <= 1000)
        {
            uint32 level = action - 100;
            AddRandomAffixesForLevel(player, level);
            return AddPagedData(player, creature, 0);
        }
        else
        {
            AddRandomAfixes(player);
            return AddPagedData(player, creature, pagedData.currentPage);
        }
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_RANDOM_AFFIXES_FOR_LEVEL)
    {
        AddRandomAffixesForLevel(player, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->randomMythicLevel);
        return AddPagedData(player, creature, pagedData.currentPage);
    }

    return GossipSupport::TakePagedDataAction(player, creature, action);
}

/*static*/ bool MythicPlusNpcSupport::CompareIdentifierById(const Identifier* a, const Identifier* b)
{
    return a->id < b->id;
}

uint32 MythicPlusNpcSupport::_PageZeroSender(const PagedData& pagedData) const
{
    if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVELS
        || pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST
        || pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_ALL_LEVELS
        || pagedData.type == GossipSupport::PAGED_DATA_TYPE_RANDOM_AFFIXES)
        return GOSSIP_SENDER_MAIN;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVEL_INFO)
        return GOSSIP_SENDER_MAIN + 9;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT)
        return GOSSIP_SENDER_MAIN + 10;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT_RUNS)
        return GOSSIP_SENDER_MAIN + 11;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_SNAPSHOT_DETAILS)
        return GOSSIP_SENDER_MAIN + 12;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_RANDOM_AFFIXES_FOR_LEVEL)
        return GOSSIP_SENDER_MAIN + 13;

    return GOSSIP_SENDER_MAIN;
}

bool MythicPlusNpcSupport::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    PagedData& pagedData = GetPagedData(player);
    if (sender <= GOSSIP_SENDER_MAIN + 2)
        return GossipSupport::OnGossipSelect(player, creature, sender, action);
    else if (sender == GOSSIP_SENDER_MAIN + 9)
    {
        AddMythicPlusLevels(player);
        return AddPagedData(player, creature, 0);
    }
    else if (sender == GOSSIP_SENDER_MAIN + 10)
    {
        AddMythicPlusAllLevels(player);
        return AddPagedData(player, creature, 0);
    }
    else if (sender == GOSSIP_SENDER_MAIN + 11)
    {
        AddMythicPlusDungeonListForSnapshots(player, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel);
        return AddPagedData(player, creature, 0);
    }
    else if (sender == GOSSIP_SENDER_MAIN + 12)
    {
        AddMythicPlusSnapshotAllRuns(player, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mapEntry);
        return AddPagedData(player, creature, 0);
    }
    else if (sender == GOSSIP_SENDER_MAIN + 13)
    {
        AddRandomAfixes(player);
        return AddPagedData(player, creature, 0);
    }

    return false;
}

<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.11.2" name="SoldierIdle" tilewidth="24" tileheight="24" tilecount="6" columns="6">
 <grid orientation="orthogonal" width="16" height="16"/>
 <properties>
  <property name="IdleAnimation" type="int" value="0"/>
  <property name="MoveAnimation" type="int" value="3"/>
 </properties>
 <image source="../images/units/Soldier2.png" width="144" height="24"/>
 <tile id="0">
  <animation>
   <frame tileid="0" duration="400"/>
   <frame tileid="1" duration="400"/>
   <frame tileid="2" duration="400"/>
   <frame tileid="1" duration="400"/>
  </animation>
 </tile>
 <tile id="3">
  <animation>
   <frame tileid="3" duration="100"/>
   <frame tileid="4" duration="100"/>
   <frame tileid="5" duration="100"/>
   <frame tileid="4" duration="100"/>
  </animation>
 </tile>
</tileset>

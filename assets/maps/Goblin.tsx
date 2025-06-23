<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.11.2" name="goblin" tilewidth="64" tileheight="64" tilecount="8" columns="4">
 <properties>
  <property name="IdleAnimation" type="int" value="0"/>
  <property name="MoveAnimation" type="int" value="1"/>
  <property name="UsedAnimation" type="int" value="3"/>
 </properties>
 <image source="../images/units/goblin.png" width="256" height="128"/>
 <tile id="0">
  <animation>
   <frame tileid="0" duration="400"/>
   <frame tileid="4" duration="200"/>
   <frame tileid="5" duration="400"/>
   <frame tileid="4" duration="200"/>
  </animation>
 </tile>
 <tile id="1">
  <animation>
   <frame tileid="0" duration="200"/>
   <frame tileid="1" duration="100"/>
   <frame tileid="2" duration="200"/>
   <frame tileid="1" duration="200"/>
  </animation>
 </tile>
</tileset>

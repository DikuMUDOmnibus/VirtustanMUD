#90600
��������� ����~
2 c 100
���������~
if !%arg.contains(�����)%
  wsend %actor% &Y� ��� ������?&n
  return 1
  halt
end
wait 1
wsend %actor% &Y�������� �� ���� ������, ��� ������� ���������� �������� ���������.&n
wechoaround %actor% &Y������ � ������ ������, %actor.name% ���������%actor.g% �������� ���������.&n
wait 2s
wecho &Y�������� �������� ������� � ��-��� ����������� ������� ������ ��������� ����.&n
wecho &Y�������� ��� ������������ ������ ������ �� ��������������� ������.&n
wecho &Y������ ����� ��������� ������� �������� ���.&n
wdoor 90605 north flags a
wdoor 90605 north room  90606wdoor 90606 south   flags a
wdoor 90606 south   room  90605
detach 90600 %self.id%
~
#90601
�����~
2 f 100
~
wait 1
wdoor 90605 north purge
wdoor 90606 south   purge
calcuid qroom 90605 room
attach 90600 %qroom.id%
~
#90602
��������� ����~
2 e 100
~
wait 1s
if %exist.mob(90601)%
  calcuid tenvolka 90601 mob
  %teleport% %tenvolka% 90601
  %echo% &R�����-�� ���� ��������� ��������� ������� ��� ����.&n
  wait 1s
  %force% %tenvolka% ���� .%actor.name%
end
~
#90603
��������� ���� 2~
2 e 100
~
wait 1s
if %exist.mob(90601)%
  calcuid tenvolka 90601 mob
  %teleport% %tenvolka% 90602
  %echo% &R�����-�� ���� ��������� ��������� ������� ��� ����.&n
  %force% %tenvolka% ���� .%actor.name%
end
~
#90604
��������� ���� 3~
2 e 100
~
wait 1s
if %exist.mob(90601)%
  calcuid tenvolka 90601 mob
  %teleport% %tenvolka% 90603
  %echo% &R�����-�� ���� ��������� ��������� ������� ��� ����.&n
  %force% %tenvolka% ���� .%actor.name%
end
~
#90605
��������� ���� 4~
2 e 100
~
wait 1s
if %exist.mob(90601)%
  calcuid tenvolka 90601 mob
  %teleport% %tenvolka% 90604
  %echo% &R�����-�� ���� ��������� ��������� ������� ��� ����.&n
  %force% %tenvolka% ���� .%actor.name%
end
~
#90606
���� ���������~
0 v 1
~
wait 1
if (%actor.vnum% == -1)
  dg_cast '������' .%actor.name%
else
  dg_cast '������' %actor.name%
end
%send% %actor.name% &W���� ��������� ����� ������, ������� ���� �����!&n
%echoaround% %actor% &W���� ��������� ����� ������, ������� ����� %actor.dname%!&n
mecho &R�� ���������� ������� ����, � �� ��� � ���� �������!&n
mteleport %self% 90699
~
#90607
������� ����� �������~
0 f 100
~
calcuid room90600 90600 room
attach 90608 %room90600.id%
exec 90608 %room90600.id%
detach 90607 %self.id%
~
#90608
��� ������ ���� ����� �������~
2 z 100
~
wait 1
if %exist.mob(90602)%
  calcuid duxvolka 90602 mob
  %teleport% %duxvolka% 90600
  wecho &W���������� ������ ������, ������ ������� ���� � ��������&n &R������ ���!&n
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho &R������ ���&n &Y����������� � ������� �� ����� �����!&n
  %teleport% %duxvolka% 90605
end
~
#90609
��������� ������ ��� ������~
0 q 100
~
wait 1
���� ���.���
dg_cast '���� ����'  ����
dg_cast '���� ����'  ����
dg_cast '���� ����'  ����
dg_cast '������� ����'  ����
dg_cast '������� ����'  ����
dg_cast '������� ����'  ����
��� ���
������ ���� ����� .%actor.name%
~
#90610
������� ����� �������~
0 f 100
~
calcuid room90601 90601 room
attach 90611 %room90601.id%
exec 90611 %room90601.id%
detach 90610 %self.id%
~
#90611
������ ��� ������ ���� ����� �������~
2 z 100
~
wait 1
if %exist.mob(90602)%
  calcuid duxvolka 90602 mob
  %teleport% %duxvolka% 90601
  wecho &W���������� ������ ������, ������ ������� ���� � ��������&n &R������ ���!&n
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho &R������ ���&n &Y����������� � ������� �� ����� �����!&n
  %teleport% %duxvolka% 90605
end
~
#90612
����� ����� ����~
0 f 100
~
calcuid room90602 90602 room
attach 90613 %room90602.id%
exec 90613 %room90602.id%
detach 90612 %self.id%
~
#90613
��� ������ ���� ������ �����~
2 z 100
~
wait 1
if %exist.mob(90602)%
  calcuid duxvolka 90602 mob
  %teleport% %duxvolka% 90602
  wecho &W���������� ������ ������, ������ ������� ���� � ��������&n &R������ ���!&n
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho &R������ ���&n &Y����������� � ������� �� ����� �����!&n
  %teleport% %duxvolka% 90605
end
~
#90614
����� ����� ����~
0 f 100
~
calcuid room90603 90603 room
attach 90615 %room90603.id%
exec 90615 %room90603.id%
detach 90614 %self.id%
~
#90615
��� ������ ���� ������ �����~
2 z 100
~
wait 1
if %exist.mob(90602)%
  calcuid duxvolka 90602 mob
  %teleport% %duxvolka% 90603
  wecho &W���������� ������ ������, ������ ������� ���� � ��������&n &R������ ���!&n
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho &R������ ���&n &Y����������� � ������� �� ����� �����!&n
  %teleport% %duxvolka% 90605
end
~
#90616
����� ����� ������~
0 f 100
~
calcuid room90604 90604 room
attach 90617 %room90604.id%
exec 90617 %room90604.id%
detach 90616 %self.id%
~
#90617
��� ������ ���� ������ ������~
2 z 100
~
wait 1
if %exist.mob(90602)%
  calcuid duxvolka 90602 mob
  %teleport% %duxvolka% 90604
  wecho &W���������� ������ ������, ������ ������� ���� � ��������&n &R������ ���!&n
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho &R������ ���&n &Y����������� � ������� �� ����� �����!&n
  %teleport% %duxvolka% 90605
end
~
#90618
��������� ����� � ������~
2 c 1
� �� ��� ���� ����� ������ ������� ��������~
%echoaround% &W%actor% %actor.iname% ������%actor.g% : '%arg%'&n
%send% %actor% �� ������� : '&W%arg%&n'
wecho &W%arg%!&n
wecho &W%arg%! %arg%!&n
wecho &W%arg%! %arg%! %arg%!&n
%send% %actor.name% &y����� �� ����� ��� ����������� ����������� ����!&n
%echoaround% %actor% &y����� �� ���&n &R%actor.rname%&n &y����������� ����������� ����!&n
wait 2s
wecho &R��� ����� ������� ���-�� ������������ � ��������� �������� ������, ������������� ����!&n
wait 2s
foreach mouse %self.npc%
  %force% %mouse% �����
  %force% %mouse% �����
  %force% %mouse% ���� !��� ���!
done
~
#90619
���������� ����� � ������~
2 c 1
�� ��� ���� ����� ������ �������~
%echoaround% %actor% %actor.iname% �������%actor.g% ������ : '&W%arg%&W'
%send% %actor% �� �������� ������ : '&W%arg%&n'
wecho &W%arg%!&n
wecho &W%arg%! %arg%!&n
wecho &W%arg%! %arg%! %arg%!&n
%send% %actor.name% &y����� �� ����� ��� ����������� ����������� ����!&n
%echoaround% %actor% &y����� �� ���&n &R%actor.rname%&n &y����������� ����������� ����!&n
wait 2s
wecho &R��� ����� ������� ���-�� ������������ � ��������� �������� ������, ������������� ����!&n
wait 2s
foreach mouse %self.npc%
  %force% %mouse% �����
  %force% %mouse% �����
  %force% %mouse% ���� !��� ���!
done
~
#90620
��������� ����� � ��~
2 c 1
�� ��� ���� ����� ������ �������~
%echoaround% %actor% %actor.iname% ������� : &R'%arg%'.&n
%send% %actor% �� ������� : &R'%arg%'.&n
wecho &W%arg%!&n
wecho &W%arg%! %arg%!&n
wecho &W%arg%! %arg%! %arg%!&n
%send% %actor.name% &y����� �� ����� ��� ����������� ����������� ����!&n
%echoaround% %actor% &y����� �� ���&n &R%actor.rname%&n &y����������� ����������� ����!&n
wait 2s
wecho &R��� ����� ������� ���-�� ������������ � ��������� �������� ������, ������������� ����!&n
wait 2s
foreach mouse %self.npc%
  %force% %mouse% �����
  %force% %mouse% �����
  %force% %mouse% ���� !��� ���!
done
~
#90621
transform ��������� ��������� � ������� ��������~
0 ab 100
~
if (%weather.sunlight% == �������) && (%lastlight% != �������)
  eval lastlight �������
  global lastlight
  mecho &g���������� ���� �������� ���� ���������� � ��������� ��� �� ����� %self.rname%.&n
  mecho &g������� ����������� ���� �������� �������� ��� ������� ��������.&n
  mecho &g������ � ������ %self.rname% ������ � �� �� �������� ��������������� ����� ������.&n
  mtransform 90619
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == �����) && (%lastlight% != �����)
  eval lastlight �����
  global lastlight
  mecho &g���������� ���� �������� ���� ������� ������������ � ���� %self.rname%.&n
  mecho &g������� ���������� ����������� ���� ������������ ��� ������� ��������.&n
  mecho &g����� %self.rname% ��������� �������� ������� � �� �� �������� ��������������� ����� ������.&n
  mtransform 90620
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == ����) && (%lastlight% != ����)
  eval lastlight ����
  global lastlight
  mecho &g���������� ���� �������� ���� ������� ������������ � ���� %self.rname%.&n
  mecho &g������� ���������� ����������� ���� ������������ ��� ������� ��������.&n
  mecho &g����� %self.rname% ��������� �������� ������� � �� �� �������� ��������������� ����� ������.&n
  mtransform 90620
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == ����) && (%lastlight% != ����)
  eval lastlight ����
  global lastlight
  mecho &g���������� ���� �������� ���� ���������� � ��������� ��� �� ����� %self.rname%.&n
  mecho &g������� ����������� ���� �������� �������� ��� ������� ��������.&n
  mecho &g������ � ������ %self.rname% ������ � �� �� �������� ��������������� ����� ������.&n
  mtransform 90619
  mecho &R����� ���� �������� ������ %self.name%.&n
end
~
#90622
������ ���� � ���� ������~
0 f 100
~
mload obj 90601
eval tmp %world.obj(90601)%
%tmp.put(%self.room%)%
%echo% &Y����������� ���� %self.rname% ������� � ����� �����!&n
%echo% &Y�������������� �����&n &R���� �����&n &Y������� �� ������������ ���� �����.&n
~
#90623
������� ������ ������� ������� �����~
0 k 100
~
%echo% &R%self.name% ��������%self.g% ����� ����� � �������%self.g% ���� ������ ������� ��� �������.&n
calcuidall list 90622 mob
foreach blekmouce %list%
  %echo% &R%blekmouce.iname% ������� �� ��� ����� %self.rname%.&n
  %teleport% %blekmouce% %self.name%
  %force% %blekmouce% ���� %actor.name%
done
detach 90623 %self.id%
~
#90624
������ ���� � ����� ������~
0 f 100
~
mload obj 90604
eval tmp %world.obj(90604)%
%tmp.put(%self.room%)%
%echo% &Y����������� ���� %self.rname% ������� � ����� �����!&n
%echo% &Y�������������� �����&n &R����� �����&n &Y������� �� ������������ ���� �����.&n
~
#90625
�������� ���� �����~
1 g 100
~
if !%actor.haveobj(90602)%
  %echoaround% %actor% &R%actor.iname%&n &Y������%actor.g% �� �������� � ����%actor.g% ��������&n &R�������� �� ����� ���� �����.&n
  %send% %actor.name% &Y�� ������� �� �������� � ����� ��������&n &R�������� �� ����� ���� �����.&n
  %echoaround% %actor% &Y�� ������&n &R�������%actor.g% ���� � ����� �� ����� �����&n &Y� �� ������%actor.g%, �� �����.&n
  %send% %actor.name% &Y�� ������&n &R�������� ���� � ����� �� ����� �����&n &Y� �� �������, �� �����.&n
  return 0
  halt
end
otransform 90603
%echoaround% %actor% &R%actor.iname%&n &Y������%actor.g% �� �������� � ����%actor.g% ��������&n &R�������� �� ����� ���� �����.&n
%send% %actor.name% &Y�� ������� �� �������� � ����� ��������&n &R�������� �� ����� ���� �����.&n .
%echoaround% %actor% &R������� ���� � ����� �� ����� �����, %actor.ename%&n &Y������%actor.g% ��� �� ��������� ����� �&n &g���������� �����.&n
%send% %actor.name% &R������� ���� � ����� �� ����� �����,&n &Y�� ������� ��� �� ��������� ����� �&n &Y���������� �����.&n
calcuid kolba 90602 obj
%purge% %kolba%
detach 90625 %self.id%
~
#90626
�������� ����� �����~
1 g 100
~
if !%actor.haveobj(90602)%
  %echoaround% %actor% &R%actor.iname%&n &Y������%actor.g% �� �������� � ����%actor.g% ��������&n &R�������� �� ����� ����� �����.&n
  %send% %actor.name% &Y�� ������� �� �������� � ����� ��������&n &R�������� �� ����� ����� �����.&n
  %echoaround% %actor% &Y�� ������&n &R�������%actor.g% ���� � ����� �� ����� �����&n &Y� �� ������%actor.g%, �� �����.&n
  %send% %actor.name% &Y�� ������&n &R�������� ���� � ����� �� ����� �����&n &Y� �� �������, �� �����.&n
  return 0
  halt
end
otransform 90605
%echoaround% %actor% &R%actor.iname%&n &Y������%actor.g% �� �������� � ����%actor.g% ��������&n &R�������� �� ����� ����� �����.&n
%send% %actor.name% &Y�� ������� �� �������� � ����� ��������&n &R�������� �� ����� ����� �����.&n
%echoaround% %actor% &R������� ���� � ����� �� ����� �����, %actor.ename%&n &Y������%actor.g% ��� �� ��������� ����� �&n &g���������� �����.&n
%send% %actor.name% &R������� ���� � ����� �� ����� �����,&n &Y�� ������� ��� �� ��������� ����� �&n &Y���������� �����.&n
calcuid kolba 90602 obj
%purge% %kolba%
detach 90626 %self.id%
~
#90627
�������� ������� �����~
1 g 100
~
if !%actor.haveobj(90602)%
  %echoaround% %actor% &R%actor.iname%&n &Y������%actor.g% �� �������� � ����%actor.g% ��������&n &C�������� �� ����� ������� �����.&n
  %send% %actor.name% &Y�� ������� �� �������� � ����� ��������&n &C�������� �� ����� ������� �����.&n
  %echoaround% %actor% &Y�� ������&n &C�������%actor.g% ���� � ����� �� ����� �����&n &Y� �� ������%actor.g%, �� �����.&n
  %send% %actor.name% &Y�� ������&n &C�������� ���� � ����� �� ����� �����&n &Y� �� �������, �� �����.&n
  return 0
  halt
end
otransform 90607
%echoaround% %actor% &R%actor.iname%&n &Y������%actor.g% �� �������� � ����%actor.g% ��������&n &C�������� �� ����� ������� �����.&n
%send% %actor.name% &Y�� ������� �� �������� � ����� ��������&n &C�������� �� ����� ������� �����.&n
%echoaround% %actor% &C������� ���� � ����� �� ����� �����,&n &R%actor.ename%&n &Y������%actor.g% ��� �� ��������� ����� �&n &g���������� �����.&n
%send% %actor.name% &C������� ���� � ����� �� ����� �����,&n &Y�� ������� ��� �� ��������� ����� �&n &Y���������� �����.&n
detach 90627 %self.id%
~
#90628
������ ���� � ������� ������~
0 f 100
~
mload obj 90606
eval tmp %world.obj(90606)%
%tmp.put(%self.room%)%
%echo% &Y����������� ���� %self.rname% ������� � ����� �����!&n
%echo% &Y�������������� �����&n &C������� �����&n &Y������� �� ������������ ���� �����.&n
~
#90629
transform ���������� ��������� � ������ ��������~
0 ab 100
~
if (%weather.sunlight% == �������) && (%lastlight% != �������)
  eval lastlight �������
  global lastlight
  mecho &g���������� ���� �������� ���� ���������� � ��������� ��� �� ����� %self.rname%.&n
  mecho &g������� ����������� ���� �������� �������� ��� ������� ��������.&n
  mecho &g������ � ������ %self.rname% ������ � �� �� �������� ��������������� ����� ������.&n
  mtransform 90645
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == �����) && (%lastlight% != �����)
  eval lastlight �����
  global lastlight
  mecho &g���������� ���� �������� ���� ������� ������������ � ���� %self.rname%.&n
  mecho &g������� ���������� ����������� ���� ������������ ��� ������� ��������.&n
  mecho &g����� %self.rname% ��������� �������� ������� � �� �� �������� ��������������� ����� ������.&n
  mtransform 90646
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == ����) && (%lastlight% != ����)
  eval lastlight ����
  global lastlight
  mecho &g���������� ���� �������� ���� ������� ������������ � ���� %self.rname%.&n
  mecho &g������� ���������� ����������� ���� ������������ ��� ������� ��������.&n
  mecho &g����� %self.rname% ��������� �������� ������� � �� �� �������� ��������������� ����� ������.&n
  mtransform 90646
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == ����) && (%lastlight% != ����)
  eval lastlight ����
  global lastlight
  mecho &g���������� ���� �������� ���� ���������� � ��������� ��� �� ����� %self.rname%.&n
  mecho &g������� ����������� ���� �������� �������� ��� ������� ��������.&n
  mecho &g������ � ������ %self.rname% ������ � �� �� �������� ��������������� ����� ������.&n
  mtransform 90645
  mecho &R����� ���� �������� ������ %self.name%.&n
end
~
#90630
transform ��������� ����� � ��������� ��������~
0 ab 100
~
if (%weather.sunlight% == �������) && (%lastlight% != �������)
  eval lastlight �������
  global lastlight
  mecho &g���������� ���� �������� ���� ���������� � ��������� ��� �� ����� %self.rname%.&n
  mecho &g������� ����������� ���� �������� �������� ��� ������� ��������.&n
  mecho &g������ � ������ %self.rname% ������ � �� �� �������� ��������������� ����� ������.&n
  mtransform 90647
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == �����) && (%lastlight% != �����)
  eval lastlight �����
  global lastlight
  mecho &g���������� ���� �������� ���� ������� ������������ � ���� %self.rname%.&n
  mecho &g������� ���������� ����������� ���� ������������ ��� ������� ��������.&n
  mecho &g����� %self.rname% ��������� �������� ������� � �� �� �������� ��������������� ����� ������.&n
  mtransform 90648
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == ����) && (%lastlight% != ����)
  eval lastlight ����
  global lastlight
  mecho &g���������� ���� �������� ���� ������� ������������ � ���� %self.rname%.&n
  mecho &g������� ���������� ����������� ���� ������������ ��� ������� ��������.&n
  mecho &g����� %self.rname% ��������� �������� ������� � �� �� �������� ��������������� ����� ������.&n
  mtransform 90648
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == ����) && (%lastlight% != ����)
  eval lastlight ����
  global lastlight
  mecho &g���������� ���� �������� ���� ���������� � ��������� ��� �� ����� %self.rname%.&n
  mecho &g������� ����������� ���� �������� �������� ��� ������� ��������.&n
  mecho &g������ � ������ %self.rname% ������ � �� �� �������� ��������������� ����� ������.&n
  mtransform 90647
  mecho &R����� ���� �������� ������ %self.name%.&n
end
~
#90631
transform ��������� ����� � ��������� ��������~
0 ab 100
~
if (%weather.sunlight% == �������) && (%lastlight% != �������)
  eval lastlight �������
  global lastlight
  mecho &g���������� ���� �������� ���� ���������� � ��������� ��� �� ����� %self.rname%.&n
  mecho &g������� ����������� ���� �������� �������� ��� ������� ��������.&n
  mecho &g������ � ������ %self.rname% ������ � �� �� �������� ��������������� ����� ������.&n
  mtransform 90649
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == �����) && (%lastlight% != �����)
  eval lastlight �����
  global lastlight
  mecho &g���������� ���� �������� ���� ������� ������������ � ���� %self.rname%.&n
  mecho &g������� ���������� ����������� ���� ������������ ��� ������� ��������.&n
  mecho &g����� %self.rname% ��������� �������� ������� � �� �� �������� ��������������� ����� ������.&n
  mtransform 90650
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == ����) && (%lastlight% != ����)
  eval lastlight ����
  global lastlight
  mecho &g���������� ���� �������� ���� ������� ������������ � ���� %self.rname%.&n
  mecho &g������� ���������� ����������� ���� ������������ ��� ������� ��������.&n
  mecho &g����� %self.rname% ��������� �������� ������� � �� �� �������� ��������������� ����� ������.&n
  mtransform 90650
  mecho &R����� ���� �������� ������ %self.name%.&n
end
if (%weather.sunlight% == ����) && (%lastlight% != ����)
  eval lastlight ����
  global lastlight
  mecho &g���������� ���� �������� ���� ���������� � ��������� ��� �� ����� %self.rname%.&n
  mecho &g������� ����������� ���� �������� �������� ��� ������� ��������.&n
  mecho &g������ � ������ %self.rname% ������ � �� �� �������� ��������������� ����� ������.&n
  mtransform 90649
  mecho &R����� ���� �������� ������ %self.name%.&n
end
~
#90632
������� ������� ����~
0 k 100
~
%echo% &R%self.name% ��������%self.g% ����� ����� � �������%self.g% ���� ������ ��� ������ ����.&n
calcuidall list 90626 mob
foreach zmea %list%
  %echo% &R%zmea.iname% ��������� �� ��� ����� %self.rname%.&n
  %teleport% %zmea% %self.name%
  %force% %zmea% ���� %actor.name%
done
detach 90632 %self.id%
~
#90633
������� ������� ������~
0 k 100
~
%echo% &R%self.name% ������%self.g% ����� ����� � �������%self.g% ���� ������ ��� � ����� ���.&n
calcuidall list 90628 mob
foreach pantera %list%
  %echo% &R%pantera.iname% ��������� �� ��� ����� %self.rname%.&n
  %teleport% %pantera% %self.name%
  %force% %pantera% ���� %actor.name%
done
detach 90633 %self.id%
~
#90634
���� ������~
0 k 100
~
if %random.100% < 30
  dg_affect %actor.name% ���������� ���������� 2 2
  msend %actor.name% _&R%self.name%&n &W������������ ��������� ������� �� ��� ���� �� �������, � �� ������� �� ����� ����������� ����&n
  mechoaround %actor% _&R%self.name%&n &W������������ ��������� ������� ���� �� ������� ��&n &R%actor.vname%&n &W� ��%actor.g% �����%actor.q% �� �����, ����������� ����!&n
end
~
#90635
���� ������ � ����������~
0 b 50
~
wait 1
set victim %random.pc%
if !%victim%
  halt
end
if (%random.100% <= 70)
  msend %victim% &W��������� ���� ����������� �� ����� ������,&n &C� �� ������� ����� � �� ������� �������!&n
  msend %victim% &C������� ����� ���������� ���� ��������&n, &W� �� ������� �������� �����  ����� ��� ���� � ������� ����.&n
  mechoaround %victim% &W��������� ���� ����������� �� ����� ������,&n &C� %victim.name% ������%victim.g% ����� � �� ������� �������!&n 
  mechoaround %victim% &C������� ����� ������� %victim.vname%&n, &W� ��%victim.g% ������%victim.g% ������%victim.w% �����  ����� ��� ���� � ������� ����.&n
  dg_affect %victim% ���������� ���������� 2 30
  mteleport %victim% 90691
  mechoaround %victim% &C������� ������, ���-�� ������������ %victim.vname%,&n &W������� � ����� �����, ������ ���� ����� ������� ����.&n
end
~
#90636
������ ���� ���������~
0 k 100
~
wait 5s
calcuid room90607 90607 room
attach 90637 %room90607.id%
exec 90637 %room90607.id%
~
#90637
��� ���������~
2 z 100
~
wait 1
if %exist.mob(90610)% && %exist.mob(90651)%
  calcuid szla 90651 mob
  wait 4s
  %teleport% %szla% 90607
  wecho &W� ���� ���������� ��������� ����, � ��� ���� ������� ���������&n &R���� ���!&n
  wait 1s
  %force% %szla% ���� .%actor.name%
end
detach 90637 %self.id%
~
#90638
���� ��� ������~
0 v 1
~
wait 1
if (%actor.vnum% == -1)
  dg_cast '��� ����' .%actor.name%
else
  dg_cast '��� ����' .%actor.name%
end
%send% %actor.name% &R���� ���&n &r������� ��� � ������ �� ���, � ����� ���������� ��� ���� �������� ����!&n
%echoaround% %actor% &R���� ���&n &r�������&n &R%actor.dname%&n &r� ������ �� ���, � ����� ���������� �� ������ ��� ��������� ����!&n
wait 1s
dg_cast '���' ������
mecho &r������ ��������� �����&n &��������� ������ ����, R���� ���&n &r���������� ������ �����.&n
mteleport %self% 90699
attach 90636 %self.id%
~
$~

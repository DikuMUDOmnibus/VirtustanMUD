#90600
��������� ����~
2 c 100
���������~
if !%arg.contains(�����)%
  wsend %actor% � ��� ������?
  return 1
  halt
end
wait 1
wsend %actor% �������� �� ���� ������, ��� ������� ���������� �������� ���������.
wechoaround %actor% ������ � ������ ������, %actor.name% ���������%actor.g% �������� ���������.
wait 2s
wecho �������� �������� ������� � ��-��� ����������� ������� ������ ��������� ����.
wecho �������� ��� ������������ ������ ������ �� ��������������� ������.
wecho ������ ����� ��������� ������� �������� ���.
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
  %echo% �����-�� ���� ��������� ��������� ������� ��� ����.
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
  %echo% �����-�� ���� ��������� ��������� ������� ��� ����
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
  %echo% �����-�� ���� ��������� ��������� ������� ��� ����
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
  %echo% �����-�� ���� ��������� ��������� ������� ��� ����
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
%send% %actor.name% ���� ��������� ����� ������, ������� ���� �����!
%echoaround% %actor% ���� ��������� ����� ������, ������� ����� %actor.dname%!
mecho �� ���������� ������� ����, � �� ��� � ���� �������.
mteleport %self% 90699
~
#90607
������� �������~
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
  wecho ���������� ������ ������, ������ ������� ���� � �������� ������ ���!
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho ������ ��� ����������� � ������� �� ����� �����!
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
  wecho ���������� ������ ������, ������ ������� ���� � �������� ������ ���!
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho ������ ��� ����������� � ������� �� ����� �����!
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
  wecho ���������� ������ ������, ������ ������� ���� � �������� ������ ���!
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho ������ ��� ����������� � ������� �� ����� �����!
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
  wecho ���������� ������ ������, ������ ������� ���� � �������� ������ ���!
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho ������ ��� ����������� � ������� �� ����� �����!
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
  wecho ���������� ������ ������, ������ ������� ���� � �������� ������ ���!
  wait 1s
  %force% %duxvolka% ��� ���.���
  wait 1s
  wecho ������ ��� ����������� � ������� �� ����� �����!
  %teleport% %duxvolka% 90605
end
~
#90618
��������� ����� � ������~
2 c 1
� �� ��� ���� ����� ������ ������� ��������~
%echoaround% %actor% %actor.iname% ������%actor.g% : '%arg%'
%send% %actor% �� ������� : '%arg%'
wecho %arg%!
wecho %arg%! %arg%!
wecho %arg%! %arg%! %arg%!
%send% %actor.name% ����� �� ����� ��� ����������� ����������� ����!
%echoaround% %actor% ����� �� ��� %actor.rname% ����������� ����������� ����!
wait 2s
wecho ��� ����� ������� ���-�� ������������ � ��������� �������� ������, ������������� ����!
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
%echoaround% %actor% %actor.iname% �������%actor.g% ������ : '%arg%'
%send% %actor% �� �������� ������ : '%arg%'
wecho %arg%!
wecho %arg%! %arg%!
wecho %arg%! %arg%! %arg%!
%send% %actor.name% ����� �� ����� ��� ����������� ����������� ����!
%echoaround% %actor% ����� �� ��� %actor.rname% ����������� ����������� ����!
wait 2s
wecho ��� ����� ������� ���-�� ������������ � ��������� �������� ������, ������������� ����!
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
wecho %arg%!
wecho %arg%! %arg%!
wecho %arg%! %arg%! %arg%!
%send% %actor.name% ����� �� ����� ��� ����������� ����������� ����!
%echoaround% %actor% ����� �� ��� %actor.rname% ����������� ����������� ����!
wait 2s
wecho ��� ����� ������� ���-�� ������������ � ��������� �������� ������, ������������� ����!
wait 2s
foreach mouse %self.npc%
  %force% %mouse% �����
  %force% %mouse% �����
  %force% %mouse% ���� !��� ���!
done
~
#90621
����������� � ��������~
0 b 100
~
wait 1
if (%weather.sunlight% == �������)
  ���� �������� ����
end
~
#90622
����������� � �����~
0 ab 100
~
wait 1
if (%weather.sunlight% == �����)
  ���� ��������� ����
end
~
$~

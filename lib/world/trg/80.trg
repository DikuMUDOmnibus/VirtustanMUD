#8000
����� ���� � �������~
0 q 100
~
emot ����������� �������� ���.
wait 3
say ����� ������ ������, ���� ���
if !%questor47%
  say ����... �������� �� �������� � ����� ����?
  attach 8001 %self.id%
else
  say �����...
end
~
#8001
����� �������~
0 d 100
������ ��~
say ��
if %questor47.id% ==  %actor.id%
  say �� ��� ���������� ������!
  halt
end
if %questor47%
  wait 2
  say ������, �� %questor47.vname% ��� ��������� ��� �������...
  halt
end
if (%world.curmobs(8016)% == 0)
  halt
end
set questor47 %actor%
calcuid prizrak 8016 mob
attach 8015 %prizrak.id%
global questor47
remote questor47 %prizrak.id%
say ��
.�
~
#8002
����� ��������~
0 f 0
~
if %actor.vnum% == -1
  set killer47 %actor%
else
  set killer47 %actor.leader%
end
global killer47
calcuid hunter 8000 mob
remote killer47 %hunter.id%
mload obj 8002
~
#8003
�������~
0 j 0
~
wait 1
if %object.vnum% != 8002
  say �����  ���?
  drop all
  halt
end
wait 1
mjunk all
if %actor.id% != %questor47.id%
  say � �� ������ ��� � ������...
  ������ .%actor.name%
  detach 8001 %self.id%
  halt
end
if %actor.id% != %killer47.id%
  say �� �� ���� ���!
  ����
  detach 8003 %self.id%
  halt
end
say �������!
wait 1
say ����� ������� ��� � ���� �������������
%self.gold(5000)%
give 5000 ��� .%actor.name%
~
$~

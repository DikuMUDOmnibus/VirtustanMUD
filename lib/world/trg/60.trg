#6000
������� ������~
1 c 1
�������~
if %arg.contains(������)%
  if %world.curobjs(6018)% >= 500
    osend %actor% ����� ���� � ����, �� ���������� ��� ������� �����������!
    halt
  end
  oload obj 6018
  osend %actor% �� ������� ������ �� �����
  oechoaround %actor%   %Actor.name% ������  %actor.g% ������ �� �����.
  oforce %actor% ����� ������
  oforce %actor% ����������� ������
else
  osend %actor% ��� �� ������ �������?
end
~
#6001
������~
1 j 100
~
oforce ������ �� ������, ������� ������!
done
~
#6002
���~
1 c 1
���~
oload obj 9926
oforce %actor.name% �� ���
oforce %actor.name% ���� ���
done
~
#6005
new trigger~
0 bg 100
~
� �, � ����, &G�������&n ������!
end
~
#6099
���� ������~
1 g 100
~
if %actor.level% > 31
  halt
end
if %actor.name% != �����
  osend %actor% &M���� ������ ������� ������� ������ � �� ��������� ����&n
  oechoaround %actor% &M���� ������ ������� �������� ������ ����� %actor.name% ��������� ��� �����������.&n
  otransform 6099
  return 0
  halt
end
~
$~

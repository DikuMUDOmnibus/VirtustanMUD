* BRusMUD trigger file v1.0
#8300
Greeting~
0 r0 100
~
if (%actor.eq(����)%) = 8302 
  mecho ������ ����������� � �����: ������, ������ � �������, ������ �� �����! 
  mecho ������ �� �� ������
else  
  mecho ������ ����� ��� �����: ����������, ������ ��� ��� ������!
  mforce %actor% %-direction%
end




~
#8301
Key_receiver~
0 j0 0
~
if %object% == 8300
wait 1
mecho '������� �� �������. ������ � ����� ������� ���� ����'
mexp %actor% 10000

































~
#8302
Door_close~
2 0 0
~
wdoor 8316 ����� cd
wdoor 8318 ������ cd

































~
#8303
Door_open~
0 0 0
~

wdoor 8316 ����� purge
wdoor 8318 ������ purge

































~
#8312
speech~
0 q0 0
~
msay ����� � �����, ���� ������ ����.

























~
$
$

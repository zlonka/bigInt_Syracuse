# bigInt_Syracuse<br/>
Computes syracuse numbers sequence (n/2 if n even, n*3+1 if n odd) on big integers<br/>
Big integers are stored as a structure:<br/>
 - array of 32 bits words storing numbers up to 100000000<br/>
 - number of words<br/>
 - number of allocated words<br/><br/>
Following operations are coded fot big ints:<br/>
 - init, set, comparison, output, ...<br/>
 - inc (big++)<br/>
 - mul (bigA=bigB*bigC)<br/>
 - mulx2 (big*=2)<br/>
 - div/2 (big/=2)<br/>
 - mulDigit (big*=uint)<br/>
 - pow (big=big^uint)<br/>
 - mul digits (display product of big digits)<br/>
bigInts can be used to compute almost infinite Syracuse numbers sequence, Erdos multiplicative persistence, etc...

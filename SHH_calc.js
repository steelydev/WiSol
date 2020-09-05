function Cal() {
   
	R1 = parseFloat(document.form1.R1.value)   
	T1 = parseFloat(document.form1.T1.value) 
	R2 = parseFloat(document.form1.R2.value)
	T2 = parseFloat(document.form1.T2.value)
	R3 = parseFloat(document.form1.R3.value)
	T3 = parseFloat(document.form1.T3.value)

	T0 = 273.15 + 25
	T1 = 273.15 + T1
	T2 = 273.15 + T2
	T3 = 273.15 + T3
	
	if (R1>5 && R2>5 && R3>5 && R1!=R2 && R2!=R3 && R1!=R3 && T1!=T2) {
		Fail = 0;
		Beta = - T1 * T2 * Math.log(R1/R2) / (T1-T2);
		R25 = R1 / (Math.exp(- Beta * (T1 - T0)/ T1 / T0));
		C =((1/T1-1/T2)-(Math.log(R1)- Math.log(R2))*(1/T1-1/T3)/(Math.log(R1)-Math.log(R3)))/((Math.pow(Math.log(R1),3)-Math.pow(Math.log(R2),3)) - (Math.log(R1)-Math.log(R2))*(Math.pow(Math.log(R1),3)-Math.pow(Math.log(R3),3))/(Math.log(R1)-Math.log(R3)));
		B =((1/T1-1/T2)-C*(Math.pow(Math.log(R1),3)-Math.pow(Math.log(R2),3)))/(Math.log(R1)-Math.log(R2));
		A = 1/T1-C*(Math.log(R1))*(Math.log(R1))*(Math.log(R1))-B*Math.log(R1);
		Rmin = Math.min(R1, R2, R3);
		Rmax =Math.max(R1, R2, R3);
	} else {
		Fail = 1;
		Beta =0;
		R25 = 0;
		C = 0;
		B = 0;
		A = 0;
	}

	document.form1.Beta.value = Beta.toFixed(2);
	document.form1.R25.value = R25.toFixed(2);
	var AA = A*1e+3;
	var BB = B*1e+4;
	var CC = C*1e+7;
	document.form1.A.value = AA.toPrecision(10);
	document.form1.B.value = BB.toPrecision(10);
	document.form1.C.value = CC.toPrecision(10);


	Model_Calculator();// call thermistot calculator	
	draw();
}
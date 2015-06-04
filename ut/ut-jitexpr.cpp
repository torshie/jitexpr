#include "jit_expr.h"
#include <cassert>

int main() {
	GarbageFilterPara param;
	memset(&param, 0, sizeof(param));

	{
		JitExpr expr;
		assert(expr.compile(" 1 "));
		assert(expr.value<double>(param) == 1);
	}
	{
		JitExpr expr;
		assert(expr.compile(" (1) "));
		assert(expr.value<double>(param) == 1);
	}
	{
		JitExpr expr;
		assert(expr.compile(" 1 + 1"));
		assert(expr.value<double>(param) == 2);
	}
	{
		JitExpr expr;
		assert(expr.compile(" 2 - 1"));
		assert(expr.value<double>(param) == 1);
	}
	{
		JitExpr expr;
		assert(expr.compile(" 2 * 2"));
		assert(expr.value<double>(param) == 4);
	}
	{
		JitExpr expr;
		assert(expr.compile(" 4 / 2"));
		assert(expr.value<double>(param) == 2);
	}
	{
		JitExpr expr;
		assert(expr.compile(" 1 + (2-1)"));
		assert(expr.value<double>(param) == 2);
	}
	{
		JitExpr expr;
		assert(expr.compile("1 + 4 / 2"));
		assert(expr.value<double>(param) == 3);
	}
	{
		JitExpr expr;
		assert(expr.compile("1 + 4 + 2"));
		assert(expr.value<double>(param) == 7);
	}
	{
		JitExpr expr;
		assert(expr.compile("1 + 3 * 2 + 1"));
		assert(expr.value<double>(param) == 8);
	}
	{
		JitExpr expr;
		assert(expr.compile("1 + 4 / 2 * 3 + 2"));
		assert(expr.value<double>(param) == 9);
	}
	{
		JitExpr expr;
		assert(expr.compile("1 + 4 / 2 * (3 + 2)+(1)"));
		assert(expr.value<double>(param) == 12);
	}
	{
		JitExpr expr;
		assert(expr.compile("1==1"));
		assert(expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("1>1"));
		assert(!expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("1<2"));
		assert(expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("1>2"));
		assert(!expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("1>2 && 1==1"));
		assert(!expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("1>2 || 1==1"));
		assert(expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("$nStartTermDis"));
		param.nStartTermDis = 1;
		assert(expr.value<double>(param) == 1);
	}
	{
		JitExpr expr;
		assert(expr.compile("$nWalkDis"));
		param.nWalkDis = 2;
		assert(expr.value<double>(param) == 2);
	}
	{
		JitExpr expr;
		assert(expr.compile("$nWalkLimit"));
		param.nWalkLimit = 3;
		assert(expr.value<double>(param) == 3);
	}
	{
		JitExpr expr;
		assert(expr.compile("$nDriveDis"));
		param.nDriveDis = 4;
		assert(expr.value<double>(param) == 4);
	}
	{
		JitExpr expr;
		assert(expr.compile("$nDriveLimit"));
		param.nDriveLimit = 5;
		assert(expr.value<double>(param) == 5);
	}
	{
		JitExpr expr;
		assert(expr.compile("$nAutoDisLimit"));
		param.nAutoDisLimit = 6;
		assert(expr.value<double>(param) == 6);
	}
	{
		JitExpr expr;
		assert(expr.compile("$n1stTime"));
		param.n1stTime = 7;
		assert(expr.value<double>(param) == 7);
	}
	{
		JitExpr expr;
		assert(expr.compile("$n1stWalkDis"));
		param.n1stWalkDis = 8;
		assert(expr.value<double>(param) == 8);
	}
	{
		JitExpr expr;
		assert(expr.compile("$n1stDriveDis"));
		param.n1stDriveDis = 9;
		assert(expr.value<double>(param) == 9);
	}
	{
		JitExpr expr;
		assert(expr.compile("$d1stTransferTimes"));
		param.d1stTransferTimes = 10;
		assert(expr.value<double>(param) == 10);
	}
	{
		JitExpr expr;
		assert(expr.compile("$bCanWalk"));
		param.bCanWalk = true;
		assert(expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("$nRankPos"));
		param.nRankPos = 11;
		assert(expr.value<double>(param) == 11);
	}
	{
		JitExpr expr;
		assert(expr.compile("$bWhiteListAggresive"));
		param.bWhiteListAggresive = true;
		assert(expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("$bPureSub"));
		param.bPureSub = true;
		assert(expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("$bFindOut1stPureSub"));
		param.bFindOut1stPureSub = true;
		assert(expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("$d1stPureSubTransferTimes"));
		param.d1stPureSubTransferTimes = 12;
		assert(expr.value<double>(param) == 12);
	}
	{
		JitExpr expr;
		assert(expr.compile("$n1stPureSubWalkDis"));
		param.n1stPureSubWalkDis = 13;
		assert(expr.value<double>(param) == 13);
	}
	{
		JitExpr expr;
		assert(expr.compile("$n1stPureSubDriveDis"));
		param.n1stPureSubDriveDis = 14;
		assert(expr.value<double>(param) == 14);
	}
	{
		JitExpr expr;
		assert(expr.compile("$dTransferTimes"));
		param.dTransferTimes = 15;
		assert(expr.value<double>(param) == 15);
	}
	{
		JitExpr expr;
		assert(expr.compile("$dTransferTimes == 15"));
		assert(expr.value<bool>(param));
	}
	{
		JitExpr expr;
		assert(expr.compile("$dTransferTimes + $n1stPureSubDriveDis * $nWalkDis"));
		assert(expr.value<double>(param) == 43);
	}
	return 0;
}

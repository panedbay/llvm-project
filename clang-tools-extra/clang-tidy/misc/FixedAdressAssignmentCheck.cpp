//===--- FixedAdressAssignmentCheck.cpp - clang-tidy ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FixedAdressAssignmentCheck.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/StringRef.h"

using namespace clang::ast_matchers;

namespace clang::tidy::misc {

void FixedAdressAssignmentCheck::registerMatchers(MatchFinder *Finder) {
  auto PointerConstantExprMatcher =
  expr(
      hasType(pointerType()),
      
      unless(ignoringParenImpCasts(cxxNullPtrLiteralExpr())),

      anyOf(
            hasDescendant(integerLiteral().bind("I")),
            anything()
      )
  );


Finder->addMatcher(
  varDecl(hasInitializer(PointerConstantExprMatcher.bind("E")))
      .bind("varDecl"),
  this);
Finder->addMatcher(
  fieldDecl(hasInClassInitializer(PointerConstantExprMatcher.bind("E")))
      .bind("fieldDecl"),
  this);


Finder->addMatcher(
  binaryOperator(
      hasOperatorName("="),
      hasRHS(PointerConstantExprMatcher.bind("E"))
  ).bind("assignment"),
  this);

 
Finder->addMatcher(
  initListExpr(hasDescendant(PointerConstantExprMatcher.bind("E"))).bind("assignment"),
  this);

  Finder->addMatcher(
    callExpr(
      hasAnyArgument(PointerConstantExprMatcher.bind("argExpr"))
    ).bind("callExpr"),
    this);
    


Finder->addMatcher(
  returnStmt(
      hasReturnValue(PointerConstantExprMatcher.bind("E"))
  ).bind("returnStmt"),
  this);


Finder->addMatcher(
  initListExpr(
      hasType(arrayType(hasElementType(pointerType()))),
      forEach(expr(PointerConstantExprMatcher.bind("E")))
  ).bind("initList"),
  this);

}

void FixedAdressAssignmentCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *E = Result.Nodes.getNodeAs<Expr>("E");
  const auto *I = Result.Nodes.getNodeAs<IntegerLiteral>("I");
  
  
  Expr::EvalResult Res;
  ASTContext &Context = *Result.Context;
  


  if (const auto *I = Result.Nodes.getNodeAs<IntegerLiteral>("I")) {
    if (I->EvaluateAsInt(Res, Context)) {
      llvm::APSInt Value = Res.Val.getInt();
      if (Value != 0) {
        diag(I->getBeginLoc() ,
           "fixed adress assigned to pointer");
    }
    }
    
    
  } else {
    diag(E->getEndLoc() ,
           "fixed adress assigned to pointer");
  }

}

} // namespace clang::tidy::misc

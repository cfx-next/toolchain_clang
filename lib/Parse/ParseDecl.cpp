//===--- ParseDecl.cpp - Declaration Parsing ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Declaration portions of the Parser interfaces.
//
//===----------------------------------------------------------------------===//

#include "clang/Parse/Parser.h"
#include "RAIIObjectsForParser.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/AddressSpaces.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/PrettyDeclStackTrace.h"
#include "clang/Sema/Scope.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSwitch.h"
using namespace clang;

//===----------------------------------------------------------------------===//
// C99 6.7: Declarations.
//===----------------------------------------------------------------------===//

/// ParseTypeName
///       type-name: [C99 6.7.6]
///         specifier-qualifier-list abstract-declarator[opt]
///
/// Called type-id in C++.
TypeResult Parser::ParseTypeName(SourceRange *Range,
                                 Declarator::TheContext Context,
                                 AccessSpecifier AS,
                                 Decl **OwnedType,
                                 ParsedAttributes *Attrs) {
  DeclSpecContext DSC = getDeclSpecContextFromDeclaratorContext(Context);
  if (DSC == DSC_normal)
    DSC = DSC_type_specifier;

  // Parse the common declaration-specifiers piece.
  DeclSpec DS(AttrFactory);
  if (Attrs)
    DS.addAttributes(Attrs->getList());
  ParseSpecifierQualifierList(DS, AS, DSC);
  if (OwnedType)
    *OwnedType = DS.isTypeSpecOwned() ? DS.getRepAsDecl() : 0;

  // Parse the abstract-declarator, if present.
  Declarator DeclaratorInfo(DS, Context);
  ParseDeclarator(DeclaratorInfo);
  if (Range)
    *Range = DeclaratorInfo.getSourceRange();

  if (DeclaratorInfo.isInvalidType())
    return true;

  return Actions.ActOnTypeName(getCurScope(), DeclaratorInfo);
}


/// isAttributeLateParsed - Return true if the attribute has arguments that
/// require late parsing.
static bool isAttributeLateParsed(const IdentifierInfo &II) {
#define CLANG_ATTR_LATE_PARSED_LIST
    return llvm::StringSwitch<bool>(II.getName())
#include "clang/Parse/AttrParserStringSwitches.inc"
        .Default(false);
#undef CLANG_ATTR_LATE_PARSED_LIST
}

/// ParseGNUAttributes - Parse a non-empty attributes list.
///
/// [GNU] attributes:
///         attribute
///         attributes attribute
///
/// [GNU]  attribute:
///          '__attribute__' '(' '(' attribute-list ')' ')'
///
/// [GNU]  attribute-list:
///          attrib
///          attribute_list ',' attrib
///
/// [GNU]  attrib:
///          empty
///          attrib-name
///          attrib-name '(' identifier ')'
///          attrib-name '(' identifier ',' nonempty-expr-list ')'
///          attrib-name '(' argument-expression-list [C99 6.5.2] ')'
///
/// [GNU]  attrib-name:
///          identifier
///          typespec
///          typequal
///          storageclass
///
/// Whether an attribute takes an 'identifier' is determined by the
/// attrib-name. GCC's behavior here is not worth imitating:
///
///  * In C mode, if the attribute argument list starts with an identifier
///    followed by a ',' or an ')', and the identifier doesn't resolve to
///    a type, it is parsed as an identifier. If the attribute actually
///    wanted an expression, it's out of luck (but it turns out that no
///    attributes work that way, because C constant expressions are very
///    limited).
///  * In C++ mode, if the attribute argument list starts with an identifier,
///    and the attribute *wants* an identifier, it is parsed as an identifier.
///    At block scope, any additional tokens between the identifier and the
///    ',' or ')' are ignored, otherwise they produce a parse error.
///
/// We follow the C++ model, but don't allow junk after the identifier.
void Parser::ParseGNUAttributes(ParsedAttributes &attrs,
                                SourceLocation *endLoc,
                                LateParsedAttrList *LateAttrs,
                                Declarator *D) {
  assert(Tok.is(tok::kw___attribute) && "Not a GNU attribute list!");

  while (Tok.is(tok::kw___attribute)) {
    ConsumeToken();
    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen_after,
                         "attribute")) {
      SkipUntil(tok::r_paren, StopAtSemi); // skip until ) or ;
      return;
    }
    if (ExpectAndConsume(tok::l_paren, diag::err_expected_lparen_after, "(")) {
      SkipUntil(tok::r_paren, StopAtSemi); // skip until ) or ;
      return;
    }
    // Parse the attribute-list. e.g. __attribute__(( weak, alias("__f") ))
    while (true) {
      // Allow empty/non-empty attributes. ((__vector_size__(16),,,,))
      if (TryConsumeToken(tok::comma))
        continue;

      // Expect an identifier or declaration specifier (const, int, etc.)
      if (Tok.isNot(tok::identifier) && !isDeclarationSpecifier())
        break;

      IdentifierInfo *AttrName = Tok.getIdentifierInfo();
      SourceLocation AttrNameLoc = ConsumeToken();

      if (Tok.isNot(tok::l_paren)) {
        attrs.addNew(AttrName, AttrNameLoc, 0, AttrNameLoc, 0, 0,
                     AttributeList::AS_GNU);
        continue;
      }

      // Handle "parameterized" attributes
      if (!LateAttrs || !isAttributeLateParsed(*AttrName)) {
        ParseGNUAttributeArgs(AttrName, AttrNameLoc, attrs, endLoc, 0,
                              SourceLocation(), AttributeList::AS_GNU, D);
        continue;
      }

      // Handle attributes with arguments that require late parsing.
      LateParsedAttribute *LA =
          new LateParsedAttribute(this, *AttrName, AttrNameLoc);
      LateAttrs->push_back(LA);

      // Attributes in a class are parsed at the end of the class, along
      // with other late-parsed declarations.
      if (!ClassStack.empty() && !LateAttrs->parseSoon())
        getCurrentClass().LateParsedDeclarations.push_back(LA);

      // consume everything up to and including the matching right parens
      ConsumeAndStoreUntil(tok::r_paren, LA->Toks, true, false);

      Token Eof;
      Eof.startToken();
      Eof.setLocation(Tok.getLocation());
      LA->Toks.push_back(Eof);
    }

    if (ExpectAndConsume(tok::r_paren))
      SkipUntil(tok::r_paren, StopAtSemi);
    SourceLocation Loc = Tok.getLocation();
    if (ExpectAndConsume(tok::r_paren))
      SkipUntil(tok::r_paren, StopAtSemi);
    if (endLoc)
      *endLoc = Loc;
  }
}

/// \brief Normalizes an attribute name by dropping prefixed and suffixed __.
static StringRef normalizeAttrName(StringRef Name) {
  if (Name.size() >= 4 && Name.startswith("__") && Name.endswith("__"))
    Name = Name.drop_front(2).drop_back(2);
  return Name;
}

/// \brief Determine whether the given attribute has an identifier argument.
static bool attributeHasIdentifierArg(const IdentifierInfo &II) {
#define CLANG_ATTR_IDENTIFIER_ARG_LIST
  return llvm::StringSwitch<bool>(normalizeAttrName(II.getName()))
#include "clang/Parse/AttrParserStringSwitches.inc"
           .Default(false);
#undef CLANG_ATTR_IDENTIFIER_ARG_LIST
}

/// \brief Determine whether the given attribute parses a type argument.
static bool attributeIsTypeArgAttr(const IdentifierInfo &II) {
#define CLANG_ATTR_TYPE_ARG_LIST
  return llvm::StringSwitch<bool>(normalizeAttrName(II.getName()))
#include "clang/Parse/AttrParserStringSwitches.inc"
           .Default(false);
#undef CLANG_ATTR_TYPE_ARG_LIST
}

/// \brief Determine whether the given attribute requires parsing its arguments
/// in an unevaluated context or not.
static bool attributeParsedArgsUnevaluated(const IdentifierInfo &II) {
#define CLANG_ATTR_ARG_CONTEXT_LIST
  return llvm::StringSwitch<bool>(normalizeAttrName(II.getName()))
#include "clang/Parse/AttrParserStringSwitches.inc"
           .Default(false);
#undef CLANG_ATTR_ARG_CONTEXT_LIST
}

IdentifierLoc *Parser::ParseIdentifierLoc() {
  assert(Tok.is(tok::identifier) && "expected an identifier");
  IdentifierLoc *IL = IdentifierLoc::create(Actions.Context,
                                            Tok.getLocation(),
                                            Tok.getIdentifierInfo());
  ConsumeToken();
  return IL;
}

void Parser::ParseAttributeWithTypeArg(IdentifierInfo &AttrName,
                                       SourceLocation AttrNameLoc,
                                       ParsedAttributes &Attrs,
                                       SourceLocation *EndLoc) {
  BalancedDelimiterTracker Parens(*this, tok::l_paren);
  Parens.consumeOpen();

  TypeResult T;
  if (Tok.isNot(tok::r_paren))
    T = ParseTypeName();

  if (Parens.consumeClose())
    return;

  if (T.isInvalid())
    return;

  if (T.isUsable())
    Attrs.addNewTypeAttr(&AttrName,
                         SourceRange(AttrNameLoc, Parens.getCloseLocation()), 0,
                         AttrNameLoc, T.get(), AttributeList::AS_GNU);
  else
    Attrs.addNew(&AttrName, SourceRange(AttrNameLoc, Parens.getCloseLocation()),
                 0, AttrNameLoc, 0, 0, AttributeList::AS_GNU);
}

/// Parse the arguments to a parameterized GNU attribute or
/// a C++11 attribute in "gnu" namespace.
void Parser::ParseGNUAttributeArgs(IdentifierInfo *AttrName,
                                   SourceLocation AttrNameLoc,
                                   ParsedAttributes &Attrs,
                                   SourceLocation *EndLoc,
                                   IdentifierInfo *ScopeName,
                                   SourceLocation ScopeLoc,
                                   AttributeList::Syntax Syntax,
                                   Declarator *D) {

  assert(Tok.is(tok::l_paren) && "Attribute arg list not starting with '('");

  AttributeList::Kind AttrKind =
      AttributeList::getKind(AttrName, ScopeName, Syntax);

  // Availability attributes have their own grammar.
  // FIXME: All these cases fail to pass in the syntax and scope, and might be
  // written as C++11 gnu:: attributes.
  if (AttrKind == AttributeList::AT_Availability) {
    ParseAvailabilityAttribute(*AttrName, AttrNameLoc, Attrs, EndLoc);
    return;
  }

  if (AttrKind == AttributeList::AT_ObjCBridgeRelated) {
    ParseObjCBridgeRelatedAttribute(*AttrName, AttrNameLoc, Attrs, EndLoc);
    return;
  }

  // Type safety attributes have their own grammar.
  if (AttrKind == AttributeList::AT_TypeTagForDatatype) {
    ParseTypeTagForDatatypeAttribute(*AttrName, AttrNameLoc, Attrs, EndLoc);
    return;
  }

  // Some attributes expect solely a type parameter.
  if (attributeIsTypeArgAttr(*AttrName)) {
    ParseAttributeWithTypeArg(*AttrName, AttrNameLoc, Attrs, EndLoc);
    return;
  }

  // These may refer to the function arguments, but need to be parsed early to
  // participate in determining whether it's a redeclaration.
  llvm::OwningPtr<ParseScope> PrototypeScope;
  if (AttrName->isStr("enable_if") && D && D->isFunctionDeclarator()) {
    DeclaratorChunk::FunctionTypeInfo FTI = D->getFunctionTypeInfo();
    PrototypeScope.reset(new ParseScope(this, Scope::FunctionPrototypeScope |
                                        Scope::FunctionDeclarationScope |
                                        Scope::DeclScope));
    for (unsigned i = 0; i != FTI.NumParams; ++i) {
      ParmVarDecl *Param = cast<ParmVarDecl>(FTI.Params[i].Param);
      Actions.ActOnReenterCXXMethodParameter(getCurScope(), Param);
    }
  }

  // Ignore the left paren location for now.
  ConsumeParen();

  ArgsVector ArgExprs;

  if (Tok.is(tok::identifier)) {
    // If this attribute wants an 'identifier' argument, make it so.
    bool IsIdentifierArg = attributeHasIdentifierArg(*AttrName);

    // If we don't know how to parse this attribute, but this is the only
    // token in this argument, assume it's meant to be an identifier.
    if (AttrKind == AttributeList::UnknownAttribute ||
        AttrKind == AttributeList::IgnoredAttribute) {
      const Token &Next = NextToken();
      IsIdentifierArg = Next.is(tok::r_paren) || Next.is(tok::comma);
    }

    if (IsIdentifierArg)
      ArgExprs.push_back(ParseIdentifierLoc());
  }

  if (!ArgExprs.empty() ? Tok.is(tok::comma) : Tok.isNot(tok::r_paren)) {
    // Eat the comma.
    if (!ArgExprs.empty())
      ConsumeToken();

    // Parse the non-empty comma-separated list of expressions.
    do {
      OwningPtr<EnterExpressionEvaluationContext> Unevaluated;
      if (attributeParsedArgsUnevaluated(*AttrName))
        Unevaluated.reset(new EnterExpressionEvaluationContext(Actions,
        Sema::Unevaluated));

      ExprResult ArgExpr(ParseAssignmentExpression());
      if (ArgExpr.isInvalid()) {
        SkipUntil(tok::r_paren, StopAtSemi);
        return;
      }
      ArgExprs.push_back(ArgExpr.release());
      // Eat the comma, move to the next argument
    } while (TryConsumeToken(tok::comma));
  }

  SourceLocation RParen = Tok.getLocation();
  if (!ExpectAndConsume(tok::r_paren)) {
    SourceLocation AttrLoc = ScopeLoc.isValid() ? ScopeLoc : AttrNameLoc;
    Attrs.addNew(AttrName, SourceRange(AttrLoc, RParen), ScopeName, ScopeLoc,
                 ArgExprs.data(), ArgExprs.size(), Syntax);
  }

  if (EndLoc)
    *EndLoc = RParen;
}

/// \brief Parses a single argument for a declspec, including the
/// surrounding parens.
void Parser::ParseMicrosoftDeclSpecWithSingleArg(IdentifierInfo *AttrName,
                                                 SourceLocation AttrNameLoc,
                                                 ParsedAttributes &Attrs)
{
  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.expectAndConsume(diag::err_expected_lparen_after,
                         AttrName->getNameStart(), tok::r_paren))
    return;

  ExprResult ArgExpr(ParseConstantExpression());
  if (ArgExpr.isInvalid()) {
    T.skipToEnd();
    return;
  }
  ArgsUnion ExprList = ArgExpr.take();
  Attrs.addNew(AttrName, AttrNameLoc, 0, AttrNameLoc, &ExprList, 1,
               AttributeList::AS_Declspec);

  T.consumeClose();
}

/// \brief Determines whether a declspec is a "simple" one requiring no
/// arguments.
bool Parser::IsSimpleMicrosoftDeclSpec(IdentifierInfo *Ident) {
  return llvm::StringSwitch<bool>(Ident->getName())
    .Case("dllimport", true)
    .Case("dllexport", true)
    .Case("noreturn", true)
    .Case("nothrow", true)
    .Case("noinline", true)
    .Case("naked", true)
    .Case("appdomain", true)
    .Case("process", true)
    .Case("jitintrinsic", true)
    .Case("noalias", true)
    .Case("restrict", true)
    .Case("novtable", true)
    .Case("selectany", true)
    .Case("thread", true)
    .Case("safebuffers", true )
    .Default(false);
}

/// \brief Attempts to parse a declspec which is not simple (one that takes
/// parameters).  Will return false if we properly handled the declspec, or
/// true if it is an unknown declspec.
void Parser::ParseComplexMicrosoftDeclSpec(IdentifierInfo *Ident,
                                           SourceLocation Loc,
                                           ParsedAttributes &Attrs) {
  // Try to handle the easy case first -- these declspecs all take a single
  // parameter as their argument.
  if (llvm::StringSwitch<bool>(Ident->getName())
      .Case("uuid", true)
      .Case("align", true)
      .Case("allocate", true)
      .Default(false)) {
    ParseMicrosoftDeclSpecWithSingleArg(Ident, Loc, Attrs);
  } else if (Ident->getName() == "deprecated") {
    // The deprecated declspec has an optional single argument, so we will
    // check for a l-paren to decide whether we should parse an argument or
    // not.
    if (Tok.getKind() == tok::l_paren)
      ParseMicrosoftDeclSpecWithSingleArg(Ident, Loc, Attrs);
    else
      Attrs.addNew(Ident, Loc, 0, Loc, 0, 0, AttributeList::AS_Declspec);
  } else if (Ident->getName() == "property") {
    // The property declspec is more complex in that it can take one or two
    // assignment expressions as a parameter, but the lhs of the assignment
    // must be named get or put.
    if (Tok.isNot(tok::l_paren)) {
      Diag(Tok.getLocation(), diag::err_expected_lparen_after)
        << Ident->getNameStart();
      return;
    }
    BalancedDelimiterTracker T(*this, tok::l_paren);
    T.expectAndConsume(diag::err_expected_lparen_after,
                       Ident->getNameStart(), tok::r_paren);

    enum AccessorKind {
      AK_Invalid = -1,
      AK_Put = 0, AK_Get = 1 // indices into AccessorNames
    };
    IdentifierInfo *AccessorNames[] = { 0, 0 };
    bool HasInvalidAccessor = false;

    // Parse the accessor specifications.
    while (true) {
      // Stop if this doesn't look like an accessor spec.
      if (!Tok.is(tok::identifier)) {
        // If the user wrote a completely empty list, use a special diagnostic.
        if (Tok.is(tok::r_paren) && !HasInvalidAccessor &&
            AccessorNames[AK_Put] == 0 && AccessorNames[AK_Get] == 0) {
          Diag(Loc, diag::err_ms_property_no_getter_or_putter);
          break;
        }

        Diag(Tok.getLocation(), diag::err_ms_property_unknown_accessor);
        break;
      }

      AccessorKind Kind;
      SourceLocation KindLoc = Tok.getLocation();
      StringRef KindStr = Tok.getIdentifierInfo()->getName();
      if (KindStr == "get") {
        Kind = AK_Get;
      } else if (KindStr == "put") {
        Kind = AK_Put;

      // Recover from the common mistake of using 'set' instead of 'put'.
      } else if (KindStr == "set") {
        Diag(KindLoc, diag::err_ms_property_has_set_accessor)
          << FixItHint::CreateReplacement(KindLoc, "put");
        Kind = AK_Put;

      // Handle the mistake of forgetting the accessor kind by skipping
      // this accessor.
      } else if (NextToken().is(tok::comma) || NextToken().is(tok::r_paren)) {
        Diag(KindLoc, diag::err_ms_property_missing_accessor_kind);
        ConsumeToken();
        HasInvalidAccessor = true;
        goto next_property_accessor;

      // Otherwise, complain about the unknown accessor kind.
      } else {
        Diag(KindLoc, diag::err_ms_property_unknown_accessor);
        HasInvalidAccessor = true;
        Kind = AK_Invalid;

        // Try to keep parsing unless it doesn't look like an accessor spec.
        if (!NextToken().is(tok::equal)) break;
      }

      // Consume the identifier.
      ConsumeToken();

      // Consume the '='.
      if (!TryConsumeToken(tok::equal)) {
        Diag(Tok.getLocation(), diag::err_ms_property_expected_equal)
          << KindStr;
        break;
      }

      // Expect the method name.
      if (!Tok.is(tok::identifier)) {
        Diag(Tok.getLocation(), diag::err_ms_property_expected_accessor_name);
        break;
      }

      if (Kind == AK_Invalid) {
        // Just drop invalid accessors.
      } else if (AccessorNames[Kind] != NULL) {
        // Complain about the repeated accessor, ignore it, and keep parsing.
        Diag(KindLoc, diag::err_ms_property_duplicate_accessor) << KindStr;
      } else {
        AccessorNames[Kind] = Tok.getIdentifierInfo();
      }
      ConsumeToken();

    next_property_accessor:
      // Keep processing accessors until we run out.
      if (TryConsumeToken(tok::comma))
        continue;

      // If we run into the ')', stop without consuming it.
      if (Tok.is(tok::r_paren))
        break;

      Diag(Tok.getLocation(), diag::err_ms_property_expected_comma_or_rparen);
      break;
    }

    // Only add the property attribute if it was well-formed.
    if (!HasInvalidAccessor) {
      Attrs.addNewPropertyAttr(Ident, Loc, 0, SourceLocation(),
                               AccessorNames[AK_Get], AccessorNames[AK_Put],
                               AttributeList::AS_Declspec);
    }
    T.skipToEnd();
  } else {
    // We don't recognize this as a valid declspec, but instead of creating the
    // attribute and allowing sema to warn about it, we will warn here instead.
    // This is because some attributes have multiple spellings, but we need to
    // disallow that for declspecs (such as align vs aligned).  If we made the
    // attribute, we'd have to split the valid declspec spelling logic into
    // both locations.
    Diag(Loc, diag::warn_ms_declspec_unknown) << Ident;

    // If there's an open paren, we should eat the open and close parens under
    // the assumption that this unknown declspec has parameters.
    BalancedDelimiterTracker T(*this, tok::l_paren);
    if (!T.consumeOpen())
      T.skipToEnd();
  }
}

/// [MS] decl-specifier:
///             __declspec ( extended-decl-modifier-seq )
///
/// [MS] extended-decl-modifier-seq:
///             extended-decl-modifier[opt]
///             extended-decl-modifier extended-decl-modifier-seq
void Parser::ParseMicrosoftDeclSpec(ParsedAttributes &Attrs) {
  assert(Tok.is(tok::kw___declspec) && "Not a declspec!");

  ConsumeToken();
  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.expectAndConsume(diag::err_expected_lparen_after, "__declspec",
                         tok::r_paren))
    return;

  // An empty declspec is perfectly legal and should not warn.  Additionally,
  // you can specify multiple attributes per declspec.
  while (Tok.getKind() != tok::r_paren) {
    // We expect either a well-known identifier or a generic string.  Anything
    // else is a malformed declspec.
    bool IsString = Tok.getKind() == tok::string_literal ? true : false;
    if (!IsString && Tok.getKind() != tok::identifier &&
        Tok.getKind() != tok::kw_restrict) {
      Diag(Tok, diag::err_ms_declspec_type);
      T.skipToEnd();
      return;
    }

    IdentifierInfo *AttrName;
    SourceLocation AttrNameLoc;
    if (IsString) {
      SmallString<8> StrBuffer;
      bool Invalid = false;
      StringRef Str = PP.getSpelling(Tok, StrBuffer, &Invalid);
      if (Invalid) {
        T.skipToEnd();
        return;
      }
      AttrName = PP.getIdentifierInfo(Str);
      AttrNameLoc = ConsumeStringToken();
    } else {
      AttrName = Tok.getIdentifierInfo();
      AttrNameLoc = ConsumeToken();
    }

    if (IsString || IsSimpleMicrosoftDeclSpec(AttrName))
      // If we have a generic string, we will allow it because there is no
      // documented list of allowable string declspecs, but we know they exist
      // (for instance, SAL declspecs in older versions of MSVC).
      //
      // Alternatively, if the identifier is a simple one, then it requires no
      // arguments and can be turned into an attribute directly.
      Attrs.addNew(AttrName, AttrNameLoc, 0, AttrNameLoc, 0, 0,
                   AttributeList::AS_Declspec);
    else
      ParseComplexMicrosoftDeclSpec(AttrName, AttrNameLoc, Attrs);
  }
  T.consumeClose();
}

void Parser::ParseMicrosoftTypeAttributes(ParsedAttributes &attrs) {
  // Treat these like attributes
  while (Tok.is(tok::kw___fastcall) || Tok.is(tok::kw___stdcall) ||
         Tok.is(tok::kw___thiscall) || Tok.is(tok::kw___cdecl)   ||
         Tok.is(tok::kw___ptr64) || Tok.is(tok::kw___w64) ||
         Tok.is(tok::kw___ptr32) || Tok.is(tok::kw___unaligned) ||
         Tok.is(tok::kw___sptr) || Tok.is(tok::kw___uptr)) {
    IdentifierInfo *AttrName = Tok.getIdentifierInfo();
    SourceLocation AttrNameLoc = ConsumeToken();
    attrs.addNew(AttrName, AttrNameLoc, 0, AttrNameLoc, 0, 0,
                 AttributeList::AS_Keyword);
  }
}

void Parser::ParseBorlandTypeAttributes(ParsedAttributes &attrs) {
  // Treat these like attributes
  while (Tok.is(tok::kw___pascal)) {
    IdentifierInfo *AttrName = Tok.getIdentifierInfo();
    SourceLocation AttrNameLoc = ConsumeToken();
    attrs.addNew(AttrName, AttrNameLoc, 0, AttrNameLoc, 0, 0,
                 AttributeList::AS_Keyword);
  }
}

void Parser::ParseOpenCLAttributes(ParsedAttributes &attrs) {
  // Treat these like attributes
  while (Tok.is(tok::kw___kernel)) {
    IdentifierInfo *AttrName = Tok.getIdentifierInfo();
    SourceLocation AttrNameLoc = ConsumeToken();
    attrs.addNew(AttrName, AttrNameLoc, 0, AttrNameLoc, 0, 0,
                 AttributeList::AS_Keyword);
  }
}

void Parser::ParseOpenCLQualifiers(ParsedAttributes &Attrs) {
  IdentifierInfo *AttrName = Tok.getIdentifierInfo();
  SourceLocation AttrNameLoc = Tok.getLocation();
  Attrs.addNew(AttrName, AttrNameLoc, 0, AttrNameLoc, 0, 0,
               AttributeList::AS_Keyword);
}

/// \brief Parse a version number.
///
/// version:
///   simple-integer
///   simple-integer ',' simple-integer
///   simple-integer ',' simple-integer ',' simple-integer
VersionTuple Parser::ParseVersionTuple(SourceRange &Range) {
  Range = Tok.getLocation();

  if (!Tok.is(tok::numeric_constant)) {
    Diag(Tok, diag::err_expected_version);
    SkipUntil(tok::comma, tok::r_paren,
              StopAtSemi | StopBeforeMatch | StopAtCodeCompletion);
    return VersionTuple();
  }

  // Parse the major (and possibly minor and subminor) versions, which
  // are stored in the numeric constant. We utilize a quirk of the
  // lexer, which is that it handles something like 1.2.3 as a single
  // numeric constant, rather than two separate tokens.
  SmallString<512> Buffer;
  Buffer.resize(Tok.getLength()+1);
  const char *ThisTokBegin = &Buffer[0];

  // Get the spelling of the token, which eliminates trigraphs, etc.
  bool Invalid = false;
  unsigned ActualLength = PP.getSpelling(Tok, ThisTokBegin, &Invalid);
  if (Invalid)
    return VersionTuple();

  // Parse the major version.
  unsigned AfterMajor = 0;
  unsigned Major = 0;
  while (AfterMajor < ActualLength && isDigit(ThisTokBegin[AfterMajor])) {
    Major = Major * 10 + ThisTokBegin[AfterMajor] - '0';
    ++AfterMajor;
  }

  if (AfterMajor == 0) {
    Diag(Tok, diag::err_expected_version);
    SkipUntil(tok::comma, tok::r_paren,
              StopAtSemi | StopBeforeMatch | StopAtCodeCompletion);
    return VersionTuple();
  }

  if (AfterMajor == ActualLength) {
    ConsumeToken();

    // We only had a single version component.
    if (Major == 0) {
      Diag(Tok, diag::err_zero_version);
      return VersionTuple();
    }

    return VersionTuple(Major);
  }

  if (ThisTokBegin[AfterMajor] != '.' || (AfterMajor + 1 == ActualLength)) {
    Diag(Tok, diag::err_expected_version);
    SkipUntil(tok::comma, tok::r_paren,
              StopAtSemi | StopBeforeMatch | StopAtCodeCompletion);
    return VersionTuple();
  }

  // Parse the minor version.
  unsigned AfterMinor = AfterMajor + 1;
  unsigned Minor = 0;
  while (AfterMinor < ActualLength && isDigit(ThisTokBegin[AfterMinor])) {
    Minor = Minor * 10 + ThisTokBegin[AfterMinor] - '0';
    ++AfterMinor;
  }

  if (AfterMinor == ActualLength) {
    ConsumeToken();

    // We had major.minor.
    if (Major == 0 && Minor == 0) {
      Diag(Tok, diag::err_zero_version);
      return VersionTuple();
    }

    return VersionTuple(Major, Minor);
  }

  // If what follows is not a '.', we have a problem.
  if (ThisTokBegin[AfterMinor] != '.') {
    Diag(Tok, diag::err_expected_version);
    SkipUntil(tok::comma, tok::r_paren,
              StopAtSemi | StopBeforeMatch | StopAtCodeCompletion);
    return VersionTuple();
  }

  // Parse the subminor version.
  unsigned AfterSubminor = AfterMinor + 1;
  unsigned Subminor = 0;
  while (AfterSubminor < ActualLength && isDigit(ThisTokBegin[AfterSubminor])) {
    Subminor = Subminor * 10 + ThisTokBegin[AfterSubminor] - '0';
    ++AfterSubminor;
  }

  if (AfterSubminor != ActualLength) {
    Diag(Tok, diag::err_expected_version);
    SkipUntil(tok::comma, tok::r_paren,
              StopAtSemi | StopBeforeMatch | StopAtCodeCompletion);
    return VersionTuple();
  }
  ConsumeToken();
  return VersionTuple(Major, Minor, Subminor);
}

/// \brief Parse the contents of the "availability" attribute.
///
/// availability-attribute:
///   'availability' '(' platform ',' version-arg-list, opt-message')'
///
/// platform:
///   identifier
///
/// version-arg-list:
///   version-arg
///   version-arg ',' version-arg-list
///
/// version-arg:
///   'introduced' '=' version
///   'deprecated' '=' version
///   'obsoleted' = version
///   'unavailable'
/// opt-message:
///   'message' '=' <string>
void Parser::ParseAvailabilityAttribute(IdentifierInfo &Availability,
                                        SourceLocation AvailabilityLoc,
                                        ParsedAttributes &attrs,
                                        SourceLocation *endLoc) {
  enum { Introduced, Deprecated, Obsoleted, Unknown };
  AvailabilityChange Changes[Unknown];
  ExprResult MessageExpr;

  // Opening '('.
  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.consumeOpen()) {
    Diag(Tok, diag::err_expected) << tok::l_paren;
    return;
  }

  // Parse the platform name,
  if (Tok.isNot(tok::identifier)) {
    Diag(Tok, diag::err_availability_expected_platform);
    SkipUntil(tok::r_paren, StopAtSemi);
    return;
  }
  IdentifierLoc *Platform = ParseIdentifierLoc();

  // Parse the ',' following the platform name.
  if (ExpectAndConsume(tok::comma)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return;
  }

  // If we haven't grabbed the pointers for the identifiers
  // "introduced", "deprecated", and "obsoleted", do so now.
  if (!Ident_introduced) {
    Ident_introduced = PP.getIdentifierInfo("introduced");
    Ident_deprecated = PP.getIdentifierInfo("deprecated");
    Ident_obsoleted = PP.getIdentifierInfo("obsoleted");
    Ident_unavailable = PP.getIdentifierInfo("unavailable");
    Ident_message = PP.getIdentifierInfo("message");
  }

  // Parse the set of introductions/deprecations/removals.
  SourceLocation UnavailableLoc;
  do {
    if (Tok.isNot(tok::identifier)) {
      Diag(Tok, diag::err_availability_expected_change);
      SkipUntil(tok::r_paren, StopAtSemi);
      return;
    }
    IdentifierInfo *Keyword = Tok.getIdentifierInfo();
    SourceLocation KeywordLoc = ConsumeToken();

    if (Keyword == Ident_unavailable) {
      if (UnavailableLoc.isValid()) {
        Diag(KeywordLoc, diag::err_availability_redundant)
          << Keyword << SourceRange(UnavailableLoc);
      }
      UnavailableLoc = KeywordLoc;
      continue;
    }

    if (Tok.isNot(tok::equal)) {
      Diag(Tok, diag::err_expected_after) << Keyword << tok::equal;
      SkipUntil(tok::r_paren, StopAtSemi);
      return;
    }
    ConsumeToken();
    if (Keyword == Ident_message) {
      if (Tok.isNot(tok::string_literal)) { // Also reject wide string literals.
        Diag(Tok, diag::err_expected_string_literal)
          << /*Source='availability attribute'*/2;
        SkipUntil(tok::r_paren, StopAtSemi);
        return;
      }
      MessageExpr = ParseStringLiteralExpression();
      break;
    }

    SourceRange VersionRange;
    VersionTuple Version = ParseVersionTuple(VersionRange);

    if (Version.empty()) {
      SkipUntil(tok::r_paren, StopAtSemi);
      return;
    }

    unsigned Index;
    if (Keyword == Ident_introduced)
      Index = Introduced;
    else if (Keyword == Ident_deprecated)
      Index = Deprecated;
    else if (Keyword == Ident_obsoleted)
      Index = Obsoleted;
    else
      Index = Unknown;

    if (Index < Unknown) {
      if (!Changes[Index].KeywordLoc.isInvalid()) {
        Diag(KeywordLoc, diag::err_availability_redundant)
          << Keyword
          << SourceRange(Changes[Index].KeywordLoc,
                         Changes[Index].VersionRange.getEnd());
      }

      Changes[Index].KeywordLoc = KeywordLoc;
      Changes[Index].Version = Version;
      Changes[Index].VersionRange = VersionRange;
    } else {
      Diag(KeywordLoc, diag::err_availability_unknown_change)
        << Keyword << VersionRange;
    }

  } while (TryConsumeToken(tok::comma));

  // Closing ')'.
  if (T.consumeClose())
    return;

  if (endLoc)
    *endLoc = T.getCloseLocation();

  // The 'unavailable' availability cannot be combined with any other
  // availability changes. Make sure that hasn't happened.
  if (UnavailableLoc.isValid()) {
    bool Complained = false;
    for (unsigned Index = Introduced; Index != Unknown; ++Index) {
      if (Changes[Index].KeywordLoc.isValid()) {
        if (!Complained) {
          Diag(UnavailableLoc, diag::warn_availability_and_unavailable)
            << SourceRange(Changes[Index].KeywordLoc,
                           Changes[Index].VersionRange.getEnd());
          Complained = true;
        }

        // Clear out the availability.
        Changes[Index] = AvailabilityChange();
      }
    }
  }

  // Record this attribute
  attrs.addNew(&Availability,
               SourceRange(AvailabilityLoc, T.getCloseLocation()),
               0, AvailabilityLoc,
               Platform,
               Changes[Introduced],
               Changes[Deprecated],
               Changes[Obsoleted],
               UnavailableLoc, MessageExpr.take(),
               AttributeList::AS_GNU);
}

/// \brief Parse the contents of the "objc_bridge_related" attribute.
/// objc_bridge_related '(' related_class ',' opt-class_method ',' opt-instance_method ')'
/// related_class:
///     Identifier
///
/// opt-class_method:
///     Identifier: | <empty>
///
/// opt-instance_method:
///     Identifier | <empty>
///
void Parser::ParseObjCBridgeRelatedAttribute(IdentifierInfo &ObjCBridgeRelated,
                                SourceLocation ObjCBridgeRelatedLoc,
                                ParsedAttributes &attrs,
                                SourceLocation *endLoc) {
  // Opening '('.
  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.consumeOpen()) {
    Diag(Tok, diag::err_expected) << tok::l_paren;
    return;
  }
  
  // Parse the related class name.
  if (Tok.isNot(tok::identifier)) {
    Diag(Tok, diag::err_objcbridge_related_expected_related_class);
    SkipUntil(tok::r_paren, StopAtSemi);
    return;
  }
  IdentifierLoc *RelatedClass = ParseIdentifierLoc();
  if (ExpectAndConsume(tok::comma)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return;
  }

  // Parse optional class method name.
  IdentifierLoc *ClassMethod = 0;
  if (Tok.is(tok::identifier)) {
    ClassMethod = ParseIdentifierLoc();
    if (!TryConsumeToken(tok::colon)) {
      Diag(Tok, diag::err_objcbridge_related_selector_name);
      SkipUntil(tok::r_paren, StopAtSemi);
      return;
    }
  }
  if (!TryConsumeToken(tok::comma)) {
    if (Tok.is(tok::colon))
      Diag(Tok, diag::err_objcbridge_related_selector_name);
    else
      Diag(Tok, diag::err_expected) << tok::comma;
    SkipUntil(tok::r_paren, StopAtSemi);
    return;
  }
  
  // Parse optional instance method name.
  IdentifierLoc *InstanceMethod = 0;
  if (Tok.is(tok::identifier))
    InstanceMethod = ParseIdentifierLoc();
  else if (Tok.isNot(tok::r_paren)) {
    Diag(Tok, diag::err_expected) << tok::r_paren;
    SkipUntil(tok::r_paren, StopAtSemi);
    return;
  }
  
  // Closing ')'.
  if (T.consumeClose())
    return;
  
  if (endLoc)
    *endLoc = T.getCloseLocation();
  
  // Record this attribute
  attrs.addNew(&ObjCBridgeRelated,
               SourceRange(ObjCBridgeRelatedLoc, T.getCloseLocation()),
               0, ObjCBridgeRelatedLoc,
               RelatedClass,
               ClassMethod,
               InstanceMethod,
               AttributeList::AS_GNU);
  
}

// Late Parsed Attributes:
// See other examples of late parsing in lib/Parse/ParseCXXInlineMethods

void Parser::LateParsedDeclaration::ParseLexedAttributes() {}

void Parser::LateParsedClass::ParseLexedAttributes() {
  Self->ParseLexedAttributes(*Class);
}

void Parser::LateParsedAttribute::ParseLexedAttributes() {
  Self->ParseLexedAttribute(*this, true, false);
}

/// Wrapper class which calls ParseLexedAttribute, after setting up the
/// scope appropriately.
void Parser::ParseLexedAttributes(ParsingClass &Class) {
  // Deal with templates
  // FIXME: Test cases to make sure this does the right thing for templates.
  bool HasTemplateScope = !Class.TopLevelClass && Class.TemplateScope;
  ParseScope ClassTemplateScope(this, Scope::TemplateParamScope,
                                HasTemplateScope);
  if (HasTemplateScope)
    Actions.ActOnReenterTemplateScope(getCurScope(), Class.TagOrTemplate);

  // Set or update the scope flags.
  bool AlreadyHasClassScope = Class.TopLevelClass;
  unsigned ScopeFlags = Scope::ClassScope|Scope::DeclScope;
  ParseScope ClassScope(this, ScopeFlags, !AlreadyHasClassScope);
  ParseScopeFlags ClassScopeFlags(this, ScopeFlags, AlreadyHasClassScope);

  // Enter the scope of nested classes
  if (!AlreadyHasClassScope)
    Actions.ActOnStartDelayedMemberDeclarations(getCurScope(),
                                                Class.TagOrTemplate);
  if (!Class.LateParsedDeclarations.empty()) {
    for (unsigned i = 0, ni = Class.LateParsedDeclarations.size(); i < ni; ++i){
      Class.LateParsedDeclarations[i]->ParseLexedAttributes();
    }
  }

  if (!AlreadyHasClassScope)
    Actions.ActOnFinishDelayedMemberDeclarations(getCurScope(),
                                                 Class.TagOrTemplate);
}


/// \brief Parse all attributes in LAs, and attach them to Decl D.
void Parser::ParseLexedAttributeList(LateParsedAttrList &LAs, Decl *D,
                                     bool EnterScope, bool OnDefinition) {
  assert(LAs.parseSoon() &&
         "Attribute list should be marked for immediate parsing.");
  for (unsigned i = 0, ni = LAs.size(); i < ni; ++i) {
    if (D)
      LAs[i]->addDecl(D);
    ParseLexedAttribute(*LAs[i], EnterScope, OnDefinition);
    delete LAs[i];
  }
  LAs.clear();
}


/// \brief Finish parsing an attribute for which parsing was delayed.
/// This will be called at the end of parsing a class declaration
/// for each LateParsedAttribute. We consume the saved tokens and
/// create an attribute with the arguments filled in. We add this
/// to the Attribute list for the decl.
void Parser::ParseLexedAttribute(LateParsedAttribute &LA,
                                 bool EnterScope, bool OnDefinition) {
  // Save the current token position.
  SourceLocation OrigLoc = Tok.getLocation();

  // Append the current token at the end of the new token stream so that it
  // doesn't get lost.
  LA.Toks.push_back(Tok);
  PP.EnterTokenStream(LA.Toks.data(), LA.Toks.size(), true, false);
  // Consume the previously pushed token.
  ConsumeAnyToken(/*ConsumeCodeCompletionTok=*/true);

  ParsedAttributes Attrs(AttrFactory);
  SourceLocation endLoc;

  if (LA.Decls.size() > 0) {
    Decl *D = LA.Decls[0];
    NamedDecl *ND  = dyn_cast<NamedDecl>(D);
    RecordDecl *RD = dyn_cast_or_null<RecordDecl>(D->getDeclContext());

    // Allow 'this' within late-parsed attributes.
    Sema::CXXThisScopeRAII ThisScope(Actions, RD, /*TypeQuals=*/0,
                                     ND && ND->isCXXInstanceMember());

    if (LA.Decls.size() == 1) {
      // If the Decl is templatized, add template parameters to scope.
      bool HasTemplateScope = EnterScope && D->isTemplateDecl();
      ParseScope TempScope(this, Scope::TemplateParamScope, HasTemplateScope);
      if (HasTemplateScope)
        Actions.ActOnReenterTemplateScope(Actions.CurScope, D);

      // If the Decl is on a function, add function parameters to the scope.
      bool HasFunScope = EnterScope && D->isFunctionOrFunctionTemplate();
      ParseScope FnScope(this, Scope::FnScope|Scope::DeclScope, HasFunScope);
      if (HasFunScope)
        Actions.ActOnReenterFunctionContext(Actions.CurScope, D);

      ParseGNUAttributeArgs(&LA.AttrName, LA.AttrNameLoc, Attrs, &endLoc,
                            0, SourceLocation(), AttributeList::AS_GNU, 0);

      if (HasFunScope) {
        Actions.ActOnExitFunctionContext();
        FnScope.Exit();  // Pop scope, and remove Decls from IdResolver
      }
      if (HasTemplateScope) {
        TempScope.Exit();
      }
    } else {
      // If there are multiple decls, then the decl cannot be within the
      // function scope.
      ParseGNUAttributeArgs(&LA.AttrName, LA.AttrNameLoc, Attrs, &endLoc,
                            0, SourceLocation(), AttributeList::AS_GNU, 0);
    }
  } else {
    Diag(Tok, diag::warn_attribute_no_decl) << LA.AttrName.getName();
  }

  const AttributeList *AL = Attrs.getList();
  if (OnDefinition && AL && !AL->isCXX11Attribute() &&
      AL->isKnownToGCC())
    Diag(Tok, diag::warn_attribute_on_function_definition)
      << &LA.AttrName;

  for (unsigned i = 0, ni = LA.Decls.size(); i < ni; ++i)
    Actions.ActOnFinishDelayedAttribute(getCurScope(), LA.Decls[i], Attrs);

  if (Tok.getLocation() != OrigLoc) {
    // Due to a parsing error, we either went over the cached tokens or
    // there are still cached tokens left, so we skip the leftover tokens.
    // Since this is an uncommon situation that should be avoided, use the
    // expensive isBeforeInTranslationUnit call.
    if (PP.getSourceManager().isBeforeInTranslationUnit(Tok.getLocation(),
                                                        OrigLoc))
    while (Tok.getLocation() != OrigLoc && Tok.isNot(tok::eof))
      ConsumeAnyToken();
  }
}

void Parser::ParseTypeTagForDatatypeAttribute(IdentifierInfo &AttrName,
                                              SourceLocation AttrNameLoc,
                                              ParsedAttributes &Attrs,
                                              SourceLocation *EndLoc) {
  assert(Tok.is(tok::l_paren) && "Attribute arg list not starting with '('");

  BalancedDelimiterTracker T(*this, tok::l_paren);
  T.consumeOpen();

  if (Tok.isNot(tok::identifier)) {
    Diag(Tok, diag::err_expected) << tok::identifier;
    T.skipToEnd();
    return;
  }
  IdentifierLoc *ArgumentKind = ParseIdentifierLoc();

  if (ExpectAndConsume(tok::comma)) {
    T.skipToEnd();
    return;
  }

  SourceRange MatchingCTypeRange;
  TypeResult MatchingCType = ParseTypeName(&MatchingCTypeRange);
  if (MatchingCType.isInvalid()) {
    T.skipToEnd();
    return;
  }

  bool LayoutCompatible = false;
  bool MustBeNull = false;
  while (TryConsumeToken(tok::comma)) {
    if (Tok.isNot(tok::identifier)) {
      Diag(Tok, diag::err_expected) << tok::identifier;
      T.skipToEnd();
      return;
    }
    IdentifierInfo *Flag = Tok.getIdentifierInfo();
    if (Flag->isStr("layout_compatible"))
      LayoutCompatible = true;
    else if (Flag->isStr("must_be_null"))
      MustBeNull = true;
    else {
      Diag(Tok, diag::err_type_safety_unknown_flag) << Flag;
      T.skipToEnd();
      return;
    }
    ConsumeToken(); // consume flag
  }

  if (!T.consumeClose()) {
    Attrs.addNewTypeTagForDatatype(&AttrName, AttrNameLoc, 0, AttrNameLoc,
                                   ArgumentKind, MatchingCType.release(),
                                   LayoutCompatible, MustBeNull,
                                   AttributeList::AS_GNU);
  }

  if (EndLoc)
    *EndLoc = T.getCloseLocation();
}

/// DiagnoseProhibitedCXX11Attribute - We have found the opening square brackets
/// of a C++11 attribute-specifier in a location where an attribute is not
/// permitted. By C++11 [dcl.attr.grammar]p6, this is ill-formed. Diagnose this
/// situation.
///
/// \return \c true if we skipped an attribute-like chunk of tokens, \c false if
/// this doesn't appear to actually be an attribute-specifier, and the caller
/// should try to parse it.
bool Parser::DiagnoseProhibitedCXX11Attribute() {
  assert(Tok.is(tok::l_square) && NextToken().is(tok::l_square));

  switch (isCXX11AttributeSpecifier(/*Disambiguate*/true)) {
  case CAK_NotAttributeSpecifier:
    // No diagnostic: we're in Obj-C++11 and this is not actually an attribute.
    return false;

  case CAK_InvalidAttributeSpecifier:
    Diag(Tok.getLocation(), diag::err_l_square_l_square_not_attribute);
    return false;

  case CAK_AttributeSpecifier:
    // Parse and discard the attributes.
    SourceLocation BeginLoc = ConsumeBracket();
    ConsumeBracket();
    SkipUntil(tok::r_square);
    assert(Tok.is(tok::r_square) && "isCXX11AttributeSpecifier lied");
    SourceLocation EndLoc = ConsumeBracket();
    Diag(BeginLoc, diag::err_attributes_not_allowed)
      << SourceRange(BeginLoc, EndLoc);
    return true;
  }
  llvm_unreachable("All cases handled above.");
}

/// \brief We have found the opening square brackets of a C++11
/// attribute-specifier in a location where an attribute is not permitted, but
/// we know where the attributes ought to be written. Parse them anyway, and
/// provide a fixit moving them to the right place.
void Parser::DiagnoseMisplacedCXX11Attribute(ParsedAttributesWithRange &Attrs,
                                             SourceLocation CorrectLocation) {
  assert((Tok.is(tok::l_square) && NextToken().is(tok::l_square)) ||
         Tok.is(tok::kw_alignas));

  // Consume the attributes.
  SourceLocation Loc = Tok.getLocation();
  ParseCXX11Attributes(Attrs);
  CharSourceRange AttrRange(SourceRange(Loc, Attrs.Range.getEnd()), true);

  Diag(Loc, diag::err_attributes_not_allowed)
    << FixItHint::CreateInsertionFromRange(CorrectLocation, AttrRange)
    << FixItHint::CreateRemoval(AttrRange);
}

void Parser::DiagnoseProhibitedAttributes(ParsedAttributesWithRange &attrs) {
  Diag(attrs.Range.getBegin(), diag::err_attributes_not_allowed)
    << attrs.Range;
}

void Parser::ProhibitCXX11Attributes(ParsedAttributesWithRange &attrs) {
  AttributeList *AttrList = attrs.getList();
  while (AttrList) {
    if (AttrList->isCXX11Attribute()) {
      Diag(AttrList->getLoc(), diag::err_attribute_not_type_attr) 
        << AttrList->getName();
      AttrList->setInvalid();
    }
    AttrList = AttrList->getNext();
  }
}

/// ParseDeclaration - Parse a full 'declaration', which consists of
/// declaration-specifiers, some number of declarators, and a semicolon.
/// 'Context' should be a Declarator::TheContext value.  This returns the
/// location of the semicolon in DeclEnd.
///
///       declaration: [C99 6.7]
///         block-declaration ->
///           simple-declaration
///           others                   [FIXME]
/// [C++]   template-declaration
/// [C++]   namespace-definition
/// [C++]   using-directive
/// [C++]   using-declaration
/// [C++11/C11] static_assert-declaration
///         others... [FIXME]
///
Parser::DeclGroupPtrTy Parser::ParseDeclaration(StmtVector &Stmts,
                                                unsigned Context,
                                                SourceLocation &DeclEnd,
                                          ParsedAttributesWithRange &attrs) {
  ParenBraceBracketBalancer BalancerRAIIObj(*this);
  // Must temporarily exit the objective-c container scope for
  // parsing c none objective-c decls.
  ObjCDeclContextSwitch ObjCDC(*this);

  Decl *SingleDecl = 0;
  Decl *OwnedType = 0;
  switch (Tok.getKind()) {
  case tok::kw_template:
  case tok::kw_export:
    ProhibitAttributes(attrs);
    SingleDecl = ParseDeclarationStartingWithTemplate(Context, DeclEnd);
    break;
  case tok::kw_inline:
    // Could be the start of an inline namespace. Allowed as an ext in C++03.
    if (getLangOpts().CPlusPlus && NextToken().is(tok::kw_namespace)) {
      ProhibitAttributes(attrs);
      SourceLocation InlineLoc = ConsumeToken();
      SingleDecl = ParseNamespace(Context, DeclEnd, InlineLoc);
      break;
    }
    return ParseSimpleDeclaration(Stmts, Context, DeclEnd, attrs,
                                  true);
  case tok::kw_namespace:
    ProhibitAttributes(attrs);
    SingleDecl = ParseNamespace(Context, DeclEnd);
    break;
  case tok::kw_using:
    SingleDecl = ParseUsingDirectiveOrDeclaration(Context, ParsedTemplateInfo(),
                                                  DeclEnd, attrs, &OwnedType);
    break;
  case tok::kw_static_assert:
  case tok::kw__Static_assert:
    ProhibitAttributes(attrs);
    SingleDecl = ParseStaticAssertDeclaration(DeclEnd);
    break;
  default:
    return ParseSimpleDeclaration(Stmts, Context, DeclEnd, attrs, true);
  }

  // This routine returns a DeclGroup, if the thing we parsed only contains a
  // single decl, convert it now. Alias declarations can also declare a type;
  // include that too if it is present.
  return Actions.ConvertDeclToDeclGroup(SingleDecl, OwnedType);
}

///       simple-declaration: [C99 6.7: declaration] [C++ 7p1: dcl.dcl]
///         declaration-specifiers init-declarator-list[opt] ';'
/// [C++11] attribute-specifier-seq decl-specifier-seq[opt]
///             init-declarator-list ';'
///[C90/C++]init-declarator-list ';'                             [TODO]
/// [OMP]   threadprivate-directive                              [TODO]
///
///       for-range-declaration: [C++11 6.5p1: stmt.ranged]
///         attribute-specifier-seq[opt] type-specifier-seq declarator
///
/// If RequireSemi is false, this does not check for a ';' at the end of the
/// declaration.  If it is true, it checks for and eats it.
///
/// If FRI is non-null, we might be parsing a for-range-declaration instead
/// of a simple-declaration. If we find that we are, we also parse the
/// for-range-initializer, and place it here.
Parser::DeclGroupPtrTy
Parser::ParseSimpleDeclaration(StmtVector &Stmts, unsigned Context,
                               SourceLocation &DeclEnd,
                               ParsedAttributesWithRange &Attrs,
                               bool RequireSemi, ForRangeInit *FRI) {
  // Parse the common declaration-specifiers piece.
  ParsingDeclSpec DS(*this);

  DeclSpecContext DSContext = getDeclSpecContextFromDeclaratorContext(Context);
  ParseDeclarationSpecifiers(DS, ParsedTemplateInfo(), AS_none, DSContext);

  // If we had a free-standing type definition with a missing semicolon, we
  // may get this far before the problem becomes obvious.
  if (DS.hasTagDefinition() &&
      DiagnoseMissingSemiAfterTagDefinition(DS, AS_none, DSContext))
    return DeclGroupPtrTy();

  // C99 6.7.2.3p6: Handle "struct-or-union identifier;", "enum { X };"
  // declaration-specifiers init-declarator-list[opt] ';'
  if (Tok.is(tok::semi)) {
    ProhibitAttributes(Attrs);
    DeclEnd = Tok.getLocation();
    if (RequireSemi) ConsumeToken();
    Decl *TheDecl = Actions.ParsedFreeStandingDeclSpec(getCurScope(), AS_none,
                                                       DS);
    DS.complete(TheDecl);
    return Actions.ConvertDeclToDeclGroup(TheDecl);
  }

  DS.takeAttributesFrom(Attrs);
  return ParseDeclGroup(DS, Context, /*FunctionDefs=*/ false, &DeclEnd, FRI);
}

/// Returns true if this might be the start of a declarator, or a common typo
/// for a declarator.
bool Parser::MightBeDeclarator(unsigned Context) {
  switch (Tok.getKind()) {
  case tok::annot_cxxscope:
  case tok::annot_template_id:
  case tok::caret:
  case tok::code_completion:
  case tok::coloncolon:
  case tok::ellipsis:
  case tok::kw___attribute:
  case tok::kw_operator:
  case tok::l_paren:
  case tok::star:
    return true;

  case tok::amp:
  case tok::ampamp:
    return getLangOpts().CPlusPlus;

  case tok::l_square: // Might be an attribute on an unnamed bit-field.
    return Context == Declarator::MemberContext && getLangOpts().CPlusPlus11 &&
           NextToken().is(tok::l_square);

  case tok::colon: // Might be a typo for '::' or an unnamed bit-field.
    return Context == Declarator::MemberContext || getLangOpts().CPlusPlus;

  case tok::identifier:
    switch (NextToken().getKind()) {
    case tok::code_completion:
    case tok::coloncolon:
    case tok::comma:
    case tok::equal:
    case tok::equalequal: // Might be a typo for '='.
    case tok::kw_alignas:
    case tok::kw_asm:
    case tok::kw___attribute:
    case tok::l_brace:
    case tok::l_paren:
    case tok::l_square:
    case tok::less:
    case tok::r_brace:
    case tok::r_paren:
    case tok::r_square:
    case tok::semi:
      return true;

    case tok::colon:
      // At namespace scope, 'identifier:' is probably a typo for 'identifier::'
      // and in block scope it's probably a label. Inside a class definition,
      // this is a bit-field.
      return Context == Declarator::MemberContext ||
             (getLangOpts().CPlusPlus && Context == Declarator::FileContext);

    case tok::identifier: // Possible virt-specifier.
      return getLangOpts().CPlusPlus11 && isCXX11VirtSpecifier(NextToken());

    default:
      return false;
    }

  default:
    return false;
  }
}

/// Skip until we reach something which seems like a sensible place to pick
/// up parsing after a malformed declaration. This will sometimes stop sooner
/// than SkipUntil(tok::r_brace) would, but will never stop later.
void Parser::SkipMalformedDecl() {
  while (true) {
    switch (Tok.getKind()) {
    case tok::l_brace:
      // Skip until matching }, then stop. We've probably skipped over
      // a malformed class or function definition or similar.
      ConsumeBrace();
      SkipUntil(tok::r_brace);
      if (Tok.is(tok::comma) || Tok.is(tok::l_brace) || Tok.is(tok::kw_try)) {
        // This declaration isn't over yet. Keep skipping.
        continue;
      }
      TryConsumeToken(tok::semi);
      return;

    case tok::l_square:
      ConsumeBracket();
      SkipUntil(tok::r_square);
      continue;

    case tok::l_paren:
      ConsumeParen();
      SkipUntil(tok::r_paren);
      continue;

    case tok::r_brace:
      return;

    case tok::semi:
      ConsumeToken();
      return;

    case tok::kw_inline:
      // 'inline namespace' at the start of a line is almost certainly
      // a good place to pick back up parsing, except in an Objective-C
      // @interface context.
      if (Tok.isAtStartOfLine() && NextToken().is(tok::kw_namespace) &&
          (!ParsingInObjCContainer || CurParsedObjCImpl))
        return;
      break;

    case tok::kw_namespace:
      // 'namespace' at the start of a line is almost certainly a good
      // place to pick back up parsing, except in an Objective-C
      // @interface context.
      if (Tok.isAtStartOfLine() &&
          (!ParsingInObjCContainer || CurParsedObjCImpl))
        return;
      break;

    case tok::at:
      // @end is very much like } in Objective-C contexts.
      if (NextToken().isObjCAtKeyword(tok::objc_end) &&
          ParsingInObjCContainer)
        return;
      break;

    case tok::minus:
    case tok::plus:
      // - and + probably start new method declarations in Objective-C contexts.
      if (Tok.isAtStartOfLine() && ParsingInObjCContainer)
        return;
      break;

    case tok::eof:
    case tok::annot_module_begin:
    case tok::annot_module_end:
    case tok::annot_module_include:
      return;

    default:
      break;
    }

    ConsumeAnyToken();
  }
}

/// ParseDeclGroup - Having concluded that this is either a function
/// definition or a group of object declarations, actually parse the
/// result.
Parser::DeclGroupPtrTy Parser::ParseDeclGroup(ParsingDeclSpec &DS,
                                              unsigned Context,
                                              bool AllowFunctionDefinitions,
                                              SourceLocation *DeclEnd,
                                              ForRangeInit *FRI) {
  // Parse the first declarator.
  ParsingDeclarator D(*this, DS, static_cast<Declarator::TheContext>(Context));
  ParseDeclarator(D);

  // Bail out if the first declarator didn't seem well-formed.
  if (!D.hasName() && !D.mayOmitIdentifier()) {
    SkipMalformedDecl();
    return DeclGroupPtrTy();
  }

  // Save late-parsed attributes for now; they need to be parsed in the
  // appropriate function scope after the function Decl has been constructed.
  // These will be parsed in ParseFunctionDefinition or ParseLexedAttrList.
  LateParsedAttrList LateParsedAttrs(true);
  if (D.isFunctionDeclarator())
    MaybeParseGNUAttributes(D, &LateParsedAttrs);

  // Check to see if we have a function *definition* which must have a body.
  if (D.isFunctionDeclarator() &&
      // Look at the next token to make sure that this isn't a function
      // declaration.  We have to check this because __attribute__ might be the
      // start of a function definition in GCC-extended K&R C.
      !isDeclarationAfterDeclarator()) {

    if (AllowFunctionDefinitions) {
      if (isStartOfFunctionDefinition(D)) {
        if (DS.getStorageClassSpec() == DeclSpec::SCS_typedef) {
          Diag(Tok, diag::err_function_declared_typedef);

          // Recover by treating the 'typedef' as spurious.
          DS.ClearStorageClassSpecs();
        }

        Decl *TheDecl =
          ParseFunctionDefinition(D, ParsedTemplateInfo(), &LateParsedAttrs);
        return Actions.ConvertDeclToDeclGroup(TheDecl);
      }

      if (isDeclarationSpecifier()) {
        // If there is an invalid declaration specifier right after the function
        // prototype, then we must be in a missing semicolon case where this isn't
        // actually a body.  Just fall through into the code that handles it as a
        // prototype, and let the top-level code handle the erroneous declspec
        // where it would otherwise expect a comma or semicolon.
      } else {
        Diag(Tok, diag::err_expected_fn_body);
        SkipUntil(tok::semi);
        return DeclGroupPtrTy();
      }
    } else {
      if (Tok.is(tok::l_brace)) {
        Diag(Tok, diag::err_function_definition_not_allowed);
        SkipMalformedDecl();
        return DeclGroupPtrTy();
      }
    }
  }

  if (ParseAsmAttributesAfterDeclarator(D))
    return DeclGroupPtrTy();

  // C++0x [stmt.iter]p1: Check if we have a for-range-declarator. If so, we
  // must parse and analyze the for-range-initializer before the declaration is
  // analyzed.
  //
  // Handle the Objective-C for-in loop variable similarly, although we
  // don't need to parse the container in advance.
  if (FRI && (Tok.is(tok::colon) || isTokIdentifier_in())) {
    bool IsForRangeLoop = false;
    if (TryConsumeToken(tok::colon, FRI->ColonLoc)) {
      IsForRangeLoop = true;
      if (Tok.is(tok::l_brace))
        FRI->RangeExpr = ParseBraceInitializer();
      else
        FRI->RangeExpr = ParseExpression();
    }

    Decl *ThisDecl = Actions.ActOnDeclarator(getCurScope(), D);
    if (IsForRangeLoop)
      Actions.ActOnCXXForRangeDecl(ThisDecl);
    Actions.FinalizeDeclaration(ThisDecl);
    D.complete(ThisDecl);
    return Actions.FinalizeDeclaratorGroup(getCurScope(), DS, ThisDecl);
  }

  SmallVector<Decl *, 8> DeclsInGroup;
  Decl *FirstDecl = ParseDeclarationAfterDeclaratorAndAttributes(D);
  if (LateParsedAttrs.size() > 0)
    ParseLexedAttributeList(LateParsedAttrs, FirstDecl, true, false);
  D.complete(FirstDecl);
  if (FirstDecl)
    DeclsInGroup.push_back(FirstDecl);

  bool ExpectSemi = Context != Declarator::ForContext;
  
  // If we don't have a comma, it is either the end of the list (a ';') or an
  // error, bail out.
  SourceLocation CommaLoc;
  while (TryConsumeToken(tok::comma, CommaLoc)) {
    if (Tok.isAtStartOfLine() && ExpectSemi && !MightBeDeclarator(Context)) {
      // This comma was followed by a line-break and something which can't be
      // the start of a declarator. The comma was probably a typo for a
      // semicolon.
      Diag(CommaLoc, diag::err_expected_semi_declaration)
        << FixItHint::CreateReplacement(CommaLoc, ";");
      ExpectSemi = false;
      break;
    }

    // Parse the next declarator.
    D.clear();
    D.setCommaLoc(CommaLoc);

    // Accept attributes in an init-declarator.  In the first declarator in a
    // declaration, these would be part of the declspec.  In subsequent
    // declarators, they become part of the declarator itself, so that they
    // don't apply to declarators after *this* one.  Examples:
    //    short __attribute__((common)) var;    -> declspec
    //    short var __attribute__((common));    -> declarator
    //    short x, __attribute__((common)) var;    -> declarator
    MaybeParseGNUAttributes(D);

    ParseDeclarator(D);
    if (!D.isInvalidType()) {
      Decl *ThisDecl = ParseDeclarationAfterDeclarator(D);
      D.complete(ThisDecl);
      if (ThisDecl)
        DeclsInGroup.push_back(ThisDecl);
    }
  }

  if (DeclEnd)
    *DeclEnd = Tok.getLocation();

  if (ExpectSemi &&
      ExpectAndConsumeSemi(Context == Declarator::FileContext
                           ? diag::err_invalid_token_after_toplevel_declarator
                           : diag::err_expected_semi_declaration)) {
    // Okay, there was no semicolon and one was expected.  If we see a
    // declaration specifier, just assume it was missing and continue parsing.
    // Otherwise things are very confused and we skip to recover.
    if (!isDeclarationSpecifier()) {
      SkipUntil(tok::r_brace, StopAtSemi | StopBeforeMatch);
      TryConsumeToken(tok::semi);
    }
  }

  return Actions.FinalizeDeclaratorGroup(getCurScope(), DS, DeclsInGroup);
}

/// Parse an optional simple-asm-expr and attributes, and attach them to a
/// declarator. Returns true on an error.
bool Parser::ParseAsmAttributesAfterDeclarator(Declarator &D) {
  // If a simple-asm-expr is present, parse it.
  if (Tok.is(tok::kw_asm)) {
    SourceLocation Loc;
    ExprResult AsmLabel(ParseSimpleAsm(&Loc));
    if (AsmLabel.isInvalid()) {
      SkipUntil(tok::semi, StopBeforeMatch);
      return true;
    }

    D.setAsmLabel(AsmLabel.release());
    D.SetRangeEnd(Loc);
  }

  MaybeParseGNUAttributes(D);
  return false;
}

/// \brief Parse 'declaration' after parsing 'declaration-specifiers
/// declarator'. This method parses the remainder of the declaration
/// (including any attributes or initializer, among other things) and
/// finalizes the declaration.
///
///       init-declarator: [C99 6.7]
///         declarator
///         declarator '=' initializer
/// [GNU]   declarator simple-asm-expr[opt] attributes[opt]
/// [GNU]   declarator simple-asm-expr[opt] attributes[opt] '=' initializer
/// [C++]   declarator initializer[opt]
///
/// [C++] initializer:
/// [C++]   '=' initializer-clause
/// [C++]   '(' expression-list ')'
/// [C++0x] '=' 'default'                                                [TODO]
/// [C++0x] '=' 'delete'
/// [C++0x] braced-init-list
///
/// According to the standard grammar, =default and =delete are function
/// definitions, but that definitely doesn't fit with the parser here.
///
Decl *Parser::ParseDeclarationAfterDeclarator(Declarator &D,
                                     const ParsedTemplateInfo &TemplateInfo) {
  if (ParseAsmAttributesAfterDeclarator(D))
    return 0;

  return ParseDeclarationAfterDeclaratorAndAttributes(D, TemplateInfo);
}

Decl *Parser::ParseDeclarationAfterDeclaratorAndAttributes(Declarator &D,
                                     const ParsedTemplateInfo &TemplateInfo) {
  // Inform the current actions module that we just parsed this declarator.
  Decl *ThisDecl = 0;
  switch (TemplateInfo.Kind) {
  case ParsedTemplateInfo::NonTemplate:
    ThisDecl = Actions.ActOnDeclarator(getCurScope(), D);
    break;

  case ParsedTemplateInfo::Template:
  case ParsedTemplateInfo::ExplicitSpecialization: {
    ThisDecl = Actions.ActOnTemplateDeclarator(getCurScope(),
                                               *TemplateInfo.TemplateParams,
                                               D);
    if (VarTemplateDecl *VT = dyn_cast_or_null<VarTemplateDecl>(ThisDecl))
      // Re-direct this decl to refer to the templated decl so that we can
      // initialize it.
      ThisDecl = VT->getTemplatedDecl();
    break;
  }
  case ParsedTemplateInfo::ExplicitInstantiation: {
    if (Tok.is(tok::semi)) {
      DeclResult ThisRes = Actions.ActOnExplicitInstantiation(
          getCurScope(), TemplateInfo.ExternLoc, TemplateInfo.TemplateLoc, D);
      if (ThisRes.isInvalid()) {
        SkipUntil(tok::semi, StopBeforeMatch);
        return 0;
      }
      ThisDecl = ThisRes.get();
    } else {
      // FIXME: This check should be for a variable template instantiation only.

      // Check that this is a valid instantiation
      if (D.getName().getKind() != UnqualifiedId::IK_TemplateId) {
        // If the declarator-id is not a template-id, issue a diagnostic and
        // recover by ignoring the 'template' keyword.
        Diag(Tok, diag::err_template_defn_explicit_instantiation)
            << 2 << FixItHint::CreateRemoval(TemplateInfo.TemplateLoc);
        ThisDecl = Actions.ActOnDeclarator(getCurScope(), D);
      } else {
        SourceLocation LAngleLoc =
            PP.getLocForEndOfToken(TemplateInfo.TemplateLoc);
        Diag(D.getIdentifierLoc(),
             diag::err_explicit_instantiation_with_definition)
            << SourceRange(TemplateInfo.TemplateLoc)
            << FixItHint::CreateInsertion(LAngleLoc, "<>");

        // Recover as if it were an explicit specialization.
        TemplateParameterLists FakedParamLists;
        FakedParamLists.push_back(Actions.ActOnTemplateParameterList(
            0, SourceLocation(), TemplateInfo.TemplateLoc, LAngleLoc, 0, 0,
            LAngleLoc));

        ThisDecl =
            Actions.ActOnTemplateDeclarator(getCurScope(), FakedParamLists, D);
      }
    }
    break;
    }
  }

  bool TypeContainsAuto = D.getDeclSpec().containsPlaceholderType();

  // Parse declarator '=' initializer.
  // If a '==' or '+=' is found, suggest a fixit to '='.
  if (isTokenEqualOrEqualTypo()) {
    ConsumeToken();

    if (Tok.is(tok::kw_delete)) {
      if (D.isFunctionDeclarator())
        Diag(ConsumeToken(), diag::err_default_delete_in_multiple_declaration)
          << 1 /* delete */;
      else
        Diag(ConsumeToken(), diag::err_deleted_non_function);
    } else if (Tok.is(tok::kw_default)) {
      if (D.isFunctionDeclarator())
        Diag(ConsumeToken(), diag::err_default_delete_in_multiple_declaration)
          << 0 /* default */;
      else
        Diag(ConsumeToken(), diag::err_default_special_members);
    } else {
      if (getLangOpts().CPlusPlus && D.getCXXScopeSpec().isSet()) {
        EnterScope(0);
        Actions.ActOnCXXEnterDeclInitializer(getCurScope(), ThisDecl);
      }

      if (Tok.is(tok::code_completion)) {
        Actions.CodeCompleteInitializer(getCurScope(), ThisDecl);
        Actions.FinalizeDeclaration(ThisDecl);
        cutOffParsing();
        return 0;
      }

      ExprResult Init(ParseInitializer());

      if (getLangOpts().CPlusPlus && D.getCXXScopeSpec().isSet()) {
        Actions.ActOnCXXExitDeclInitializer(getCurScope(), ThisDecl);
        ExitScope();
      }

      if (Init.isInvalid()) {
        SkipUntil(tok::comma, StopAtSemi | StopBeforeMatch);
        Actions.ActOnInitializerError(ThisDecl);
      } else
        Actions.AddInitializerToDecl(ThisDecl, Init.take(),
                                     /*DirectInit=*/false, TypeContainsAuto);
    }
  } else if (Tok.is(tok::l_paren)) {
    // Parse C++ direct initializer: '(' expression-list ')'
    BalancedDelimiterTracker T(*this, tok::l_paren);
    T.consumeOpen();

    ExprVector Exprs;
    CommaLocsTy CommaLocs;

    if (getLangOpts().CPlusPlus && D.getCXXScopeSpec().isSet()) {
      EnterScope(0);
      Actions.ActOnCXXEnterDeclInitializer(getCurScope(), ThisDecl);
    }

    if (ParseExpressionList(Exprs, CommaLocs)) {
      Actions.ActOnInitializerError(ThisDecl);
      SkipUntil(tok::r_paren, StopAtSemi);

      if (getLangOpts().CPlusPlus && D.getCXXScopeSpec().isSet()) {
        Actions.ActOnCXXExitDeclInitializer(getCurScope(), ThisDecl);
        ExitScope();
      }
    } else {
      // Match the ')'.
      T.consumeClose();

      assert(!Exprs.empty() && Exprs.size()-1 == CommaLocs.size() &&
             "Unexpected number of commas!");

      if (getLangOpts().CPlusPlus && D.getCXXScopeSpec().isSet()) {
        Actions.ActOnCXXExitDeclInitializer(getCurScope(), ThisDecl);
        ExitScope();
      }

      ExprResult Initializer = Actions.ActOnParenListExpr(T.getOpenLocation(),
                                                          T.getCloseLocation(),
                                                          Exprs);
      Actions.AddInitializerToDecl(ThisDecl, Initializer.take(),
                                   /*DirectInit=*/true, TypeContainsAuto);
    }
  } else if (getLangOpts().CPlusPlus11 && Tok.is(tok::l_brace) &&
             (!CurParsedObjCImpl || !D.isFunctionDeclarator())) {
    // Parse C++0x braced-init-list.
    Diag(Tok, diag::warn_cxx98_compat_generalized_initializer_lists);

    if (D.getCXXScopeSpec().isSet()) {
      EnterScope(0);
      Actions.ActOnCXXEnterDeclInitializer(getCurScope(), ThisDecl);
    }

    ExprResult Init(ParseBraceInitializer());

    if (D.getCXXScopeSpec().isSet()) {
      Actions.ActOnCXXExitDeclInitializer(getCurScope(), ThisDecl);
      ExitScope();
    }

    if (Init.isInvalid()) {
      Actions.ActOnInitializerError(ThisDecl);
    } else
      Actions.AddInitializerToDecl(ThisDecl, Init.take(),
                                   /*DirectInit=*/true, TypeContainsAuto);

  } else {
    Actions.ActOnUninitializedDecl(ThisDecl, TypeContainsAuto);
  }

  Actions.FinalizeDeclaration(ThisDecl);

  return ThisDecl;
}

/// ParseSpecifierQualifierList
///        specifier-qualifier-list:
///          type-specifier specifier-qualifier-list[opt]
///          type-qualifier specifier-qualifier-list[opt]
/// [GNU]    attributes     specifier-qualifier-list[opt]
///
void Parser::ParseSpecifierQualifierList(DeclSpec &DS, AccessSpecifier AS,
                                         DeclSpecContext DSC) {
  /// specifier-qualifier-list is a subset of declaration-specifiers.  Just
  /// parse declaration-specifiers and complain about extra stuff.
  /// TODO: diagnose attribute-specifiers and alignment-specifiers.
  ParseDeclarationSpecifiers(DS, ParsedTemplateInfo(), AS, DSC);

  // Validate declspec for type-name.
  unsigned Specs = DS.getParsedSpecifiers();
  if (isTypeSpecifier(DSC) && !DS.hasTypeSpecifier()) {
    Diag(Tok, diag::err_expected_type);
    DS.SetTypeSpecError();
  } else if (Specs == DeclSpec::PQ_None && !DS.getNumProtocolQualifiers() &&
             !DS.hasAttributes()) {
    Diag(Tok, diag::err_typename_requires_specqual);
    if (!DS.hasTypeSpecifier())
      DS.SetTypeSpecError();
  }

  // Issue diagnostic and remove storage class if present.
  if (Specs & DeclSpec::PQ_StorageClassSpecifier) {
    if (DS.getStorageClassSpecLoc().isValid())
      Diag(DS.getStorageClassSpecLoc(),diag::err_typename_invalid_storageclass);
    else
      Diag(DS.getThreadStorageClassSpecLoc(),
           diag::err_typename_invalid_storageclass);
    DS.ClearStorageClassSpecs();
  }

  // Issue diagnostic and remove function specfier if present.
  if (Specs & DeclSpec::PQ_FunctionSpecifier) {
    if (DS.isInlineSpecified())
      Diag(DS.getInlineSpecLoc(), diag::err_typename_invalid_functionspec);
    if (DS.isVirtualSpecified())
      Diag(DS.getVirtualSpecLoc(), diag::err_typename_invalid_functionspec);
    if (DS.isExplicitSpecified())
      Diag(DS.getExplicitSpecLoc(), diag::err_typename_invalid_functionspec);
    DS.ClearFunctionSpecs();
  }

  // Issue diagnostic and remove constexpr specfier if present.
  if (DS.isConstexprSpecified()) {
    Diag(DS.getConstexprSpecLoc(), diag::err_typename_invalid_constexpr);
    DS.ClearConstexprSpec();
  }
}

/// isValidAfterIdentifierInDeclaratorAfterDeclSpec - Return true if the
/// specified token is valid after the identifier in a declarator which
/// immediately follows the declspec.  For example, these things are valid:
///
///      int x   [             4];         // direct-declarator
///      int x   (             int y);     // direct-declarator
///  int(int x   )                         // direct-declarator
///      int x   ;                         // simple-declaration
///      int x   =             17;         // init-declarator-list
///      int x   ,             y;          // init-declarator-list
///      int x   __asm__       ("foo");    // init-declarator-list
///      int x   :             4;          // struct-declarator
///      int x   {             5};         // C++'0x unified initializers
///
/// This is not, because 'x' does not immediately follow the declspec (though
/// ')' happens to be valid anyway).
///    int (x)
///
static bool isValidAfterIdentifierInDeclarator(const Token &T) {
  return T.is(tok::l_square) || T.is(tok::l_paren) || T.is(tok::r_paren) ||
         T.is(tok::semi) || T.is(tok::comma) || T.is(tok::equal) ||
         T.is(tok::kw_asm) || T.is(tok::l_brace) || T.is(tok::colon);
}


/// ParseImplicitInt - This method is called when we have an non-typename
/// identifier in a declspec (which normally terminates the decl spec) when
/// the declspec has no type specifier.  In this case, the declspec is either
/// malformed or is "implicit int" (in K&R and C89).
///
/// This method handles diagnosing this prettily and returns false if the
/// declspec is done being processed.  If it recovers and thinks there may be
/// other pieces of declspec after it, it returns true.
///
bool Parser::ParseImplicitInt(DeclSpec &DS, CXXScopeSpec *SS,
                              const ParsedTemplateInfo &TemplateInfo,
                              AccessSpecifier AS, DeclSpecContext DSC, 
                              ParsedAttributesWithRange &Attrs) {
  assert(Tok.is(tok::identifier) && "should have identifier");

  SourceLocation Loc = Tok.getLocation();
  // If we see an identifier that is not a type name, we normally would
  // parse it as the identifer being declared.  However, when a typename
  // is typo'd or the definition is not included, this will incorrectly
  // parse the typename as the identifier name and fall over misparsing
  // later parts of the diagnostic.
  //
  // As such, we try to do some look-ahead in cases where this would
  // otherwise be an "implicit-int" case to see if this is invalid.  For
  // example: "static foo_t x = 4;"  In this case, if we parsed foo_t as
  // an identifier with implicit int, we'd get a parse error because the
  // next token is obviously invalid for a type.  Parse these as a case
  // with an invalid type specifier.
  assert(!DS.hasTypeSpecifier() && "Type specifier checked above");

  // Since we know that this either implicit int (which is rare) or an
  // error, do lookahead to try to do better recovery. This never applies
  // within a type specifier. Outside of C++, we allow this even if the
  // language doesn't "officially" support implicit int -- we support
  // implicit int as an extension in C99 and C11.
  if (!isTypeSpecifier(DSC) && !getLangOpts().CPlusPlus &&
      isValidAfterIdentifierInDeclarator(NextToken())) {
    // If this token is valid for implicit int, e.g. "static x = 4", then
    // we just avoid eating the identifier, so it will be parsed as the
    // identifier in the declarator.
    return false;
  }

  if (getLangOpts().CPlusPlus &&
      DS.getStorageClassSpec() == DeclSpec::SCS_auto) {
    // Don't require a type specifier if we have the 'auto' storage class
    // specifier in C++98 -- we'll promote it to a type specifier.
    if (SS)
      AnnotateScopeToken(*SS, /*IsNewAnnotation*/false);
    return false;
  }

  // Otherwise, if we don't consume this token, we are going to emit an
  // error anyway.  Try to recover from various common problems.  Check
  // to see if this was a reference to a tag name without a tag specified.
  // This is a common problem in C (saying 'foo' instead of 'struct foo').
  //
  // C++ doesn't need this, and isTagName doesn't take SS.
  if (SS == 0) {
    const char *TagName = 0, *FixitTagName = 0;
    tok::TokenKind TagKind = tok::unknown;

    switch (Actions.isTagName(*Tok.getIdentifierInfo(), getCurScope())) {
      default: break;
      case DeclSpec::TST_enum:
        TagName="enum"  ; FixitTagName = "enum "  ; TagKind=tok::kw_enum ;break;
      case DeclSpec::TST_union:
        TagName="union" ; FixitTagName = "union " ;TagKind=tok::kw_union ;break;
      case DeclSpec::TST_struct:
        TagName="struct"; FixitTagName = "struct ";TagKind=tok::kw_struct;break;
      case DeclSpec::TST_interface:
        TagName="__interface"; FixitTagName = "__interface ";
        TagKind=tok::kw___interface;break;
      case DeclSpec::TST_class:
        TagName="class" ; FixitTagName = "class " ;TagKind=tok::kw_class ;break;
    }

    if (TagName) {
      IdentifierInfo *TokenName = Tok.getIdentifierInfo();
      LookupResult R(Actions, TokenName, SourceLocation(),
                     Sema::LookupOrdinaryName);

      Diag(Loc, diag::err_use_of_tag_name_without_tag)
        << TokenName << TagName << getLangOpts().CPlusPlus
        << FixItHint::CreateInsertion(Tok.getLocation(), FixitTagName);

      if (Actions.LookupParsedName(R, getCurScope(), SS)) {
        for (LookupResult::iterator I = R.begin(), IEnd = R.end();
             I != IEnd; ++I)
          Diag((*I)->getLocation(), diag::note_decl_hiding_tag_type)
            << TokenName << TagName;
      }

      // Parse this as a tag as if the missing tag were present.
      if (TagKind == tok::kw_enum)
        ParseEnumSpecifier(Loc, DS, TemplateInfo, AS, DSC_normal);
      else
        ParseClassSpecifier(TagKind, Loc, DS, TemplateInfo, AS,
                            /*EnteringContext*/ false, DSC_normal, Attrs);
      return true;
    }
  }

  // Determine whether this identifier could plausibly be the name of something
  // being declared (with a missing type).
  if (!isTypeSpecifier(DSC) &&
      (!SS || DSC == DSC_top_level || DSC == DSC_class)) {
    // Look ahead to the next token to try to figure out what this declaration
    // was supposed to be.
    switch (NextToken().getKind()) {
    case tok::l_paren: {
      // static x(4); // 'x' is not a type
      // x(int n);    // 'x' is not a type
      // x (*p)[];    // 'x' is a type
      //
      // Since we're in an error case (or the rare 'implicit int in C++' MS
      // extension), we can afford to perform a tentative parse to determine
      // which case we're in.
      TentativeParsingAction PA(*this);
      ConsumeToken();
      TPResult TPR = TryParseDeclarator(/*mayBeAbstract*/false);
      PA.Revert();

      if (TPR != TPResult::False()) {
        // The identifier is followed by a parenthesized declarator.
        // It's supposed to be a type.
        break;
      }

      // If we're in a context where we could be declaring a constructor,
      // check whether this is a constructor declaration with a bogus name.
      if (DSC == DSC_class || (DSC == DSC_top_level && SS)) {
        IdentifierInfo *II = Tok.getIdentifierInfo();
        if (Actions.isCurrentClassNameTypo(II, SS)) {
          Diag(Loc, diag::err_constructor_bad_name)
            << Tok.getIdentifierInfo() << II
            << FixItHint::CreateReplacement(Tok.getLocation(), II->getName());
          Tok.setIdentifierInfo(II);
        }
      }
      // Fall through.
    }
    case tok::comma:
    case tok::equal:
    case tok::kw_asm:
    case tok::l_brace:
    case tok::l_square:
    case tok::semi:
      // This looks like a variable or function declaration. The type is
      // probably missing. We're done parsing decl-specifiers.
      if (SS)
        AnnotateScopeToken(*SS, /*IsNewAnnotation*/false);
      return false;

    default:
      // This is probably supposed to be a type. This includes cases like:
      //   int f(itn);
      //   struct S { unsinged : 4; };
      break;
    }
  }

  // This is almost certainly an invalid type name. Let the action emit a
  // diagnostic and attempt to recover.
  ParsedType T;
  IdentifierInfo *II = Tok.getIdentifierInfo();
  if (Actions.DiagnoseUnknownTypeName(II, Loc, getCurScope(), SS, T,
                                      getLangOpts().CPlusPlus &&
                                          NextToken().is(tok::less))) {
    // The action emitted a diagnostic, so we don't have to.
    if (T) {
      // The action has suggested that the type T could be used. Set that as
      // the type in the declaration specifiers, consume the would-be type
      // name token, and we're done.
      const char *PrevSpec;
      unsigned DiagID;
      DS.SetTypeSpecType(DeclSpec::TST_typename, Loc, PrevSpec, DiagID, T,
                         Actions.getASTContext().getPrintingPolicy());
      DS.SetRangeEnd(Tok.getLocation());
      ConsumeToken();
      // There may be other declaration specifiers after this.
      return true;
    } else if (II != Tok.getIdentifierInfo()) {
      // If no type was suggested, the correction is to a keyword
      Tok.setKind(II->getTokenID());
      // There may be other declaration specifiers after this.
      return true;
    }

    // Fall through; the action had no suggestion for us.
  } else {
    // The action did not emit a diagnostic, so emit one now.
    SourceRange R;
    if (SS) R = SS->getRange();
    Diag(Loc, diag::err_unknown_typename) << Tok.getIdentifierInfo() << R;
  }

  // Mark this as an error.
  DS.SetTypeSpecError();
  DS.SetRangeEnd(Tok.getLocation());
  ConsumeToken();

  // TODO: Could inject an invalid typedef decl in an enclosing scope to
  // avoid rippling error messages on subsequent uses of the same type,
  // could be useful if #include was forgotten.
  return false;
}

/// \brief Determine the declaration specifier context from the declarator
/// context.
///
/// \param Context the declarator context, which is one of the
/// Declarator::TheContext enumerator values.
Parser::DeclSpecContext
Parser::getDeclSpecContextFromDeclaratorContext(unsigned Context) {
  if (Context == Declarator::MemberContext)
    return DSC_class;
  if (Context == Declarator::FileContext)
    return DSC_top_level;
  if (Context == Declarator::TrailingReturnContext)
    return DSC_trailing;
  if (Context == Declarator::AliasDeclContext ||
      Context == Declarator::AliasTemplateContext)
    return DSC_alias_declaration;
  return DSC_normal;
}

/// ParseAlignArgument - Parse the argument to an alignment-specifier.
///
/// FIXME: Simply returns an alignof() expression if the argument is a
/// type. Ideally, the type should be propagated directly into Sema.
///
/// [C11]   type-id
/// [C11]   constant-expression
/// [C++0x] type-id ...[opt]
/// [C++0x] assignment-expression ...[opt]
ExprResult Parser::ParseAlignArgument(SourceLocation Start,
                                      SourceLocation &EllipsisLoc) {
  ExprResult ER;
  if (isTypeIdInParens()) {
    SourceLocation TypeLoc = Tok.getLocation();
    ParsedType Ty = ParseTypeName().get();
    SourceRange TypeRange(Start, Tok.getLocation());
    ER = Actions.ActOnUnaryExprOrTypeTraitExpr(TypeLoc, UETT_AlignOf, true,
                                               Ty.getAsOpaquePtr(), TypeRange);
  } else
    ER = ParseConstantExpression();

  if (getLangOpts().CPlusPlus11)
    TryConsumeToken(tok::ellipsis, EllipsisLoc);

  return ER;
}

/// ParseAlignmentSpecifier - Parse an alignment-specifier, and add the
/// attribute to Attrs.
///
/// alignment-specifier:
/// [C11]   '_Alignas' '(' type-id ')'
/// [C11]   '_Alignas' '(' constant-expression ')'
/// [C++11] 'alignas' '(' type-id ...[opt] ')'
/// [C++11] 'alignas' '(' assignment-expression ...[opt] ')'
void Parser::ParseAlignmentSpecifier(ParsedAttributes &Attrs,
                                     SourceLocation *EndLoc) {
  assert((Tok.is(tok::kw_alignas) || Tok.is(tok::kw__Alignas)) &&
         "Not an alignment-specifier!");

  IdentifierInfo *KWName = Tok.getIdentifierInfo();
  SourceLocation KWLoc = ConsumeToken();

  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.expectAndConsume())
    return;

  SourceLocation EllipsisLoc;
  ExprResult ArgExpr = ParseAlignArgument(T.getOpenLocation(), EllipsisLoc);
  if (ArgExpr.isInvalid()) {
    T.skipToEnd();
    return;
  }

  T.consumeClose();
  if (EndLoc)
    *EndLoc = T.getCloseLocation();

  ArgsVector ArgExprs;
  ArgExprs.push_back(ArgExpr.release());
  Attrs.addNew(KWName, KWLoc, 0, KWLoc, ArgExprs.data(), 1,
               AttributeList::AS_Keyword, EllipsisLoc);
}

/// Determine whether we're looking at something that might be a declarator
/// in a simple-declaration. If it can't possibly be a declarator, maybe
/// diagnose a missing semicolon after a prior tag definition in the decl
/// specifier.
///
/// \return \c true if an error occurred and this can't be any kind of
/// declaration.
bool
Parser::DiagnoseMissingSemiAfterTagDefinition(DeclSpec &DS, AccessSpecifier AS,
                                              DeclSpecContext DSContext,
                                              LateParsedAttrList *LateAttrs) {
  assert(DS.hasTagDefinition() && "shouldn't call this");

  bool EnteringContext = (DSContext == DSC_class || DSContext == DSC_top_level);

  if (getLangOpts().CPlusPlus &&
      (Tok.is(tok::identifier) || Tok.is(tok::coloncolon) ||
       Tok.is(tok::kw_decltype) || Tok.is(tok::annot_template_id)) &&
      TryAnnotateCXXScopeToken(EnteringContext)) {
    SkipMalformedDecl();
    return true;
  }

  bool HasScope = Tok.is(tok::annot_cxxscope);
  // Make a copy in case GetLookAheadToken invalidates the result of NextToken.
  Token AfterScope = HasScope ? NextToken() : Tok;

  // Determine whether the following tokens could possibly be a
  // declarator.
  bool MightBeDeclarator = true;
  if (Tok.is(tok::kw_typename) || Tok.is(tok::annot_typename)) {
    // A declarator-id can't start with 'typename'.
    MightBeDeclarator = false;
  } else if (AfterScope.is(tok::annot_template_id)) {
    // If we have a type expressed as a template-id, this cannot be a
    // declarator-id (such a type cannot be redeclared in a simple-declaration).
    TemplateIdAnnotation *Annot =
        static_cast<TemplateIdAnnotation *>(AfterScope.getAnnotationValue());
    if (Annot->Kind == TNK_Type_template)
      MightBeDeclarator = false;
  } else if (AfterScope.is(tok::identifier)) {
    const Token &Next = HasScope ? GetLookAheadToken(2) : NextToken();

    // These tokens cannot come after the declarator-id in a
    // simple-declaration, and are likely to come after a type-specifier.
    if (Next.is(tok::star) || Next.is(tok::amp) || Next.is(tok::ampamp) ||
        Next.is(tok::identifier) || Next.is(tok::annot_cxxscope) ||
        Next.is(tok::coloncolon)) {
      // Missing a semicolon.
      MightBeDeclarator = false;
    } else if (HasScope) {
      // If the declarator-id has a scope specifier, it must redeclare a
      // previously-declared entity. If that's a type (and this is not a
      // typedef), that's an error.
      CXXScopeSpec SS;
      Actions.RestoreNestedNameSpecifierAnnotation(
          Tok.getAnnotationValue(), Tok.getAnnotationRange(), SS);
      IdentifierInfo *Name = AfterScope.getIdentifierInfo();
      Sema::NameClassification Classification = Actions.ClassifyName(
          getCurScope(), SS, Name, AfterScope.getLocation(), Next,
          /*IsAddressOfOperand*/false);
      switch (Classification.getKind()) {
      case Sema::NC_Error:
        SkipMalformedDecl();
        return true;

      case Sema::NC_Keyword:
      case Sema::NC_NestedNameSpecifier:
        llvm_unreachable("typo correction and nested name specifiers not "
                         "possible here");

      case Sema::NC_Type:
      case Sema::NC_TypeTemplate:
        // Not a previously-declared non-type entity.
        MightBeDeclarator = false;
        break;

      case Sema::NC_Unknown:
      case Sema::NC_Expression:
      case Sema::NC_VarTemplate:
      case Sema::NC_FunctionTemplate:
        // Might be a redeclaration of a prior entity.
        break;
      }
    }
  }

  if (MightBeDeclarator)
    return false;

  const PrintingPolicy &PPol = Actions.getASTContext().getPrintingPolicy();
  Diag(PP.getLocForEndOfToken(DS.getRepAsDecl()->getLocEnd()),
       diag::err_expected_after)
      << DeclSpec::getSpecifierName(DS.getTypeSpecType(), PPol) << tok::semi;

  // Try to recover from the typo, by dropping the tag definition and parsing
  // the problematic tokens as a type.
  //
  // FIXME: Split the DeclSpec into pieces for the standalone
  // declaration and pieces for the following declaration, instead
  // of assuming that all the other pieces attach to new declaration,
  // and call ParsedFreeStandingDeclSpec as appropriate.
  DS.ClearTypeSpecType();
  ParsedTemplateInfo NotATemplate;
  ParseDeclarationSpecifiers(DS, NotATemplate, AS, DSContext, LateAttrs);
  return false;
}

/// ParseDeclarationSpecifiers
///       declaration-specifiers: [C99 6.7]
///         storage-class-specifier declaration-specifiers[opt]
///         type-specifier declaration-specifiers[opt]
/// [C99]   function-specifier declaration-specifiers[opt]
/// [C11]   alignment-specifier declaration-specifiers[opt]
/// [GNU]   attributes declaration-specifiers[opt]
/// [Clang] '__module_private__' declaration-specifiers[opt]
///
///       storage-class-specifier: [C99 6.7.1]
///         'typedef'
///         'extern'
///         'static'
///         'auto'
///         'register'
/// [C++]   'mutable'
/// [C++11] 'thread_local'
/// [C11]   '_Thread_local'
/// [GNU]   '__thread'
///       function-specifier: [C99 6.7.4]
/// [C99]   'inline'
/// [C++]   'virtual'
/// [C++]   'explicit'
/// [OpenCL] '__kernel'
///       'friend': [C++ dcl.friend]
///       'constexpr': [C++0x dcl.constexpr]

///
void Parser::ParseDeclarationSpecifiers(DeclSpec &DS,
                                        const ParsedTemplateInfo &TemplateInfo,
                                        AccessSpecifier AS,
                                        DeclSpecContext DSContext,
                                        LateParsedAttrList *LateAttrs) {
  if (DS.getSourceRange().isInvalid()) {
    DS.SetRangeStart(Tok.getLocation());
    DS.SetRangeEnd(Tok.getLocation());
  }

  bool EnteringContext = (DSContext == DSC_class || DSContext == DSC_top_level);
  bool AttrsLastTime = false;
  ParsedAttributesWithRange attrs(AttrFactory);
  const PrintingPolicy &Policy = Actions.getASTContext().getPrintingPolicy();
  while (1) {
    bool isInvalid = false;
    const char *PrevSpec = 0;
    unsigned DiagID = 0;

    SourceLocation Loc = Tok.getLocation();

    switch (Tok.getKind()) {
    default:
    DoneWithDeclSpec:
      if (!AttrsLastTime)
        ProhibitAttributes(attrs);
      else {
        // Reject C++11 attributes that appertain to decl specifiers as
        // we don't support any C++11 attributes that appertain to decl
        // specifiers. This also conforms to what g++ 4.8 is doing.
        ProhibitCXX11Attributes(attrs);

        DS.takeAttributesFrom(attrs);
      }

      // If this is not a declaration specifier token, we're done reading decl
      // specifiers.  First verify that DeclSpec's are consistent.
      DS.Finish(Diags, PP, Policy);
      return;

    case tok::l_square:
    case tok::kw_alignas:
      if (!getLangOpts().CPlusPlus11 || !isCXX11AttributeSpecifier())
        goto DoneWithDeclSpec;

      ProhibitAttributes(attrs);
      // FIXME: It would be good to recover by accepting the attributes,
      //        but attempting to do that now would cause serious
      //        madness in terms of diagnostics.
      attrs.clear();
      attrs.Range = SourceRange();

      ParseCXX11Attributes(attrs);
      AttrsLastTime = true;
      continue;

    case tok::code_completion: {
      Sema::ParserCompletionContext CCC = Sema::PCC_Namespace;
      if (DS.hasTypeSpecifier()) {
        bool AllowNonIdentifiers
          = (getCurScope()->getFlags() & (Scope::ControlScope |
                                          Scope::BlockScope |
                                          Scope::TemplateParamScope |
                                          Scope::FunctionPrototypeScope |
                                          Scope::AtCatchScope)) == 0;
        bool AllowNestedNameSpecifiers
          = DSContext == DSC_top_level ||
            (DSContext == DSC_class && DS.isFriendSpecified());

        Actions.CodeCompleteDeclSpec(getCurScope(), DS,
                                     AllowNonIdentifiers,
                                     AllowNestedNameSpecifiers);
        return cutOffParsing();
      }

      if (getCurScope()->getFnParent() || getCurScope()->getBlockParent())
        CCC = Sema::PCC_LocalDeclarationSpecifiers;
      else if (TemplateInfo.Kind != ParsedTemplateInfo::NonTemplate)
        CCC = DSContext == DSC_class? Sema::PCC_MemberTemplate
                                    : Sema::PCC_Template;
      else if (DSContext == DSC_class)
        CCC = Sema::PCC_Class;
      else if (CurParsedObjCImpl)
        CCC = Sema::PCC_ObjCImplementation;

      Actions.CodeCompleteOrdinaryName(getCurScope(), CCC);
      return cutOffParsing();
    }

    case tok::coloncolon: // ::foo::bar
      // C++ scope specifier.  Annotate and loop, or bail out on error.
      if (TryAnnotateCXXScopeToken(EnteringContext)) {
        if (!DS.hasTypeSpecifier())
          DS.SetTypeSpecError();
        goto DoneWithDeclSpec;
      }
      if (Tok.is(tok::coloncolon)) // ::new or ::delete
        goto DoneWithDeclSpec;
      continue;

    case tok::annot_cxxscope: {
      if (DS.hasTypeSpecifier() || DS.isTypeAltiVecVector())
        goto DoneWithDeclSpec;

      CXXScopeSpec SS;
      Actions.RestoreNestedNameSpecifierAnnotation(Tok.getAnnotationValue(),
                                                   Tok.getAnnotationRange(),
                                                   SS);

      // We are looking for a qualified typename.
      Token Next = NextToken();
      if (Next.is(tok::annot_template_id) &&
          static_cast<TemplateIdAnnotation *>(Next.getAnnotationValue())
            ->Kind == TNK_Type_template) {
        // We have a qualified template-id, e.g., N::A<int>

        // C++ [class.qual]p2:
        //   In a lookup in which the constructor is an acceptable lookup
        //   result and the nested-name-specifier nominates a class C:
        //
        //     - if the name specified after the
        //       nested-name-specifier, when looked up in C, is the
        //       injected-class-name of C (Clause 9), or
        //
        //     - if the name specified after the nested-name-specifier
        //       is the same as the identifier or the
        //       simple-template-id's template-name in the last
        //       component of the nested-name-specifier,
        //
        //   the name is instead considered to name the constructor of
        //   class C.
        //
        // Thus, if the template-name is actually the constructor
        // name, then the code is ill-formed; this interpretation is
        // reinforced by the NAD status of core issue 635.
        TemplateIdAnnotation *TemplateId = takeTemplateIdAnnotation(Next);
        if ((DSContext == DSC_top_level || DSContext == DSC_class) &&
            TemplateId->Name &&
            Actions.isCurrentClassName(*TemplateId->Name, getCurScope(), &SS)) {
          if (isConstructorDeclarator(/*Unqualified*/false)) {
            // The user meant this to be an out-of-line constructor
            // definition, but template arguments are not allowed
            // there.  Just allow this as a constructor; we'll
            // complain about it later.
            goto DoneWithDeclSpec;
          }

          // The user meant this to name a type, but it actually names
          // a constructor with some extraneous template
          // arguments. Complain, then parse it as a type as the user
          // intended.
          Diag(TemplateId->TemplateNameLoc,
               diag::err_out_of_line_template_id_names_constructor)
            << TemplateId->Name;
        }

        DS.getTypeSpecScope() = SS;
        ConsumeToken(); // The C++ scope.
        assert(Tok.is(tok::annot_template_id) &&
               "ParseOptionalCXXScopeSpecifier not working");
        AnnotateTemplateIdTokenAsType();
        continue;
      }

      if (Next.is(tok::annot_typename)) {
        DS.getTypeSpecScope() = SS;
        ConsumeToken(); // The C++ scope.
        if (Tok.getAnnotationValue()) {
          ParsedType T = getTypeAnnotation(Tok);
          isInvalid = DS.SetTypeSpecType(DeclSpec::TST_typename,
                                         Tok.getAnnotationEndLoc(),
                                         PrevSpec, DiagID, T, Policy);
          if (isInvalid)
            break;
        }
        else
          DS.SetTypeSpecError();
        DS.SetRangeEnd(Tok.getAnnotationEndLoc());
        ConsumeToken(); // The typename
      }

      if (Next.isNot(tok::identifier))
        goto DoneWithDeclSpec;

      // If we're in a context where the identifier could be a class name,
      // check whether this is a constructor declaration.
      if ((DSContext == DSC_top_level || DSContext == DSC_class) &&
          Actions.isCurrentClassName(*Next.getIdentifierInfo(), getCurScope(),
                                     &SS)) {
        if (isConstructorDeclarator(/*Unqualified*/false))
          goto DoneWithDeclSpec;

        // As noted in C++ [class.qual]p2 (cited above), when the name
        // of the class is qualified in a context where it could name
        // a constructor, its a constructor name. However, we've
        // looked at the declarator, and the user probably meant this
        // to be a type. Complain that it isn't supposed to be treated
        // as a type, then proceed to parse it as a type.
        Diag(Next.getLocation(), diag::err_out_of_line_type_names_constructor)
          << Next.getIdentifierInfo();
      }

      ParsedType TypeRep = Actions.getTypeName(*Next.getIdentifierInfo(),
                                               Next.getLocation(),
                                               getCurScope(), &SS,
                                               false, false, ParsedType(),
                                               /*IsCtorOrDtorName=*/false,
                                               /*NonTrivialSourceInfo=*/true);

      // If the referenced identifier is not a type, then this declspec is
      // erroneous: We already checked about that it has no type specifier, and
      // C++ doesn't have implicit int.  Diagnose it as a typo w.r.t. to the
      // typename.
      if (!TypeRep) {
        ConsumeToken();   // Eat the scope spec so the identifier is current.
        ParsedAttributesWithRange Attrs(AttrFactory);
        if (ParseImplicitInt(DS, &SS, TemplateInfo, AS, DSContext, Attrs)) {
          if (!Attrs.empty()) {
            AttrsLastTime = true;
            attrs.takeAllFrom(Attrs);
          }
          continue;
        }
        goto DoneWithDeclSpec;
      }

      DS.getTypeSpecScope() = SS;
      ConsumeToken(); // The C++ scope.

      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_typename, Loc, PrevSpec,
                                     DiagID, TypeRep, Policy);
      if (isInvalid)
        break;

      DS.SetRangeEnd(Tok.getLocation());
      ConsumeToken(); // The typename.

      continue;
    }

    case tok::annot_typename: {
      // If we've previously seen a tag definition, we were almost surely
      // missing a semicolon after it.
      if (DS.hasTypeSpecifier() && DS.hasTagDefinition())
        goto DoneWithDeclSpec;

      if (Tok.getAnnotationValue()) {
        ParsedType T = getTypeAnnotation(Tok);
        isInvalid = DS.SetTypeSpecType(DeclSpec::TST_typename, Loc, PrevSpec,
                                       DiagID, T, Policy);
      } else
        DS.SetTypeSpecError();

      if (isInvalid)
        break;

      DS.SetRangeEnd(Tok.getAnnotationEndLoc());
      ConsumeToken(); // The typename

      // Objective-C supports syntax of the form 'id<proto1,proto2>' where 'id'
      // is a specific typedef and 'itf<proto1,proto2>' where 'itf' is an
      // Objective-C interface.
      if (Tok.is(tok::less) && getLangOpts().ObjC1)
        ParseObjCProtocolQualifiers(DS);

      continue;
    }

    case tok::kw___is_signed:
      // GNU libstdc++ 4.4 uses __is_signed as an identifier, but Clang
      // typically treats it as a trait. If we see __is_signed as it appears
      // in libstdc++, e.g.,
      //
      //   static const bool __is_signed;
      //
      // then treat __is_signed as an identifier rather than as a keyword.
      if (DS.getTypeSpecType() == TST_bool &&
          DS.getTypeQualifiers() == DeclSpec::TQ_const &&
          DS.getStorageClassSpec() == DeclSpec::SCS_static)
        TryKeywordIdentFallback(true);

      // We're done with the declaration-specifiers.
      goto DoneWithDeclSpec;

      // typedef-name
    case tok::kw_decltype:
    case tok::identifier: {
      // In C++, check to see if this is a scope specifier like foo::bar::, if
      // so handle it as such.  This is important for ctor parsing.
      if (getLangOpts().CPlusPlus) {
        if (TryAnnotateCXXScopeToken(EnteringContext)) {
          if (!DS.hasTypeSpecifier())
            DS.SetTypeSpecError();
          goto DoneWithDeclSpec;
        }
        if (!Tok.is(tok::identifier))
          continue;
      }

      // This identifier can only be a typedef name if we haven't already seen
      // a type-specifier.  Without this check we misparse:
      //  typedef int X; struct Y { short X; };  as 'short int'.
      if (DS.hasTypeSpecifier())
        goto DoneWithDeclSpec;

      // Check for need to substitute AltiVec keyword tokens.
      if (TryAltiVecToken(DS, Loc, PrevSpec, DiagID, isInvalid))
        break;

      // [AltiVec] 2.2: [If the 'vector' specifier is used] The syntax does not
      //                allow the use of a typedef name as a type specifier.
      if (DS.isTypeAltiVecVector())
        goto DoneWithDeclSpec;

      ParsedType TypeRep =
        Actions.getTypeName(*Tok.getIdentifierInfo(),
                            Tok.getLocation(), getCurScope());

      // If this is not a typedef name, don't parse it as part of the declspec,
      // it must be an implicit int or an error.
      if (!TypeRep) {
        ParsedAttributesWithRange Attrs(AttrFactory);
        if (ParseImplicitInt(DS, 0, TemplateInfo, AS, DSContext, Attrs)) {
          if (!Attrs.empty()) {
            AttrsLastTime = true;
            attrs.takeAllFrom(Attrs);
          }
          continue;
        }
        goto DoneWithDeclSpec;
      }

      // If we're in a context where the identifier could be a class name,
      // check whether this is a constructor declaration.
      if (getLangOpts().CPlusPlus && DSContext == DSC_class &&
          Actions.isCurrentClassName(*Tok.getIdentifierInfo(), getCurScope()) &&
          isConstructorDeclarator(/*Unqualified*/true))
        goto DoneWithDeclSpec;

      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_typename, Loc, PrevSpec,
                                     DiagID, TypeRep, Policy);
      if (isInvalid)
        break;

      DS.SetRangeEnd(Tok.getLocation());
      ConsumeToken(); // The identifier

      // Objective-C supports syntax of the form 'id<proto1,proto2>' where 'id'
      // is a specific typedef and 'itf<proto1,proto2>' where 'itf' is an
      // Objective-C interface.
      if (Tok.is(tok::less) && getLangOpts().ObjC1)
        ParseObjCProtocolQualifiers(DS);

      // Need to support trailing type qualifiers (e.g. "id<p> const").
      // If a type specifier follows, it will be diagnosed elsewhere.
      continue;
    }

      // type-name
    case tok::annot_template_id: {
      TemplateIdAnnotation *TemplateId = takeTemplateIdAnnotation(Tok);
      if (TemplateId->Kind != TNK_Type_template) {
        // This template-id does not refer to a type name, so we're
        // done with the type-specifiers.
        goto DoneWithDeclSpec;
      }

      // If we're in a context where the template-id could be a
      // constructor name or specialization, check whether this is a
      // constructor declaration.
      if (getLangOpts().CPlusPlus && DSContext == DSC_class &&
          Actions.isCurrentClassName(*TemplateId->Name, getCurScope()) &&
          isConstructorDeclarator(TemplateId->SS.isEmpty()))
        goto DoneWithDeclSpec;

      // Turn the template-id annotation token into a type annotation
      // token, then try again to parse it as a type-specifier.
      AnnotateTemplateIdTokenAsType();
      continue;
    }

    // GNU attributes support.
    case tok::kw___attribute:
      ParseGNUAttributes(DS.getAttributes(), 0, LateAttrs);
      continue;

    // Microsoft declspec support.
    case tok::kw___declspec:
      ParseMicrosoftDeclSpec(DS.getAttributes());
      continue;

    // Microsoft single token adornments.
    case tok::kw___forceinline: {
      isInvalid = DS.setFunctionSpecForceInline(Loc, PrevSpec, DiagID);
      IdentifierInfo *AttrName = Tok.getIdentifierInfo();
      SourceLocation AttrNameLoc = Tok.getLocation();
      DS.getAttributes().addNew(AttrName, AttrNameLoc, 0, AttrNameLoc, 0, 0,
                                AttributeList::AS_Keyword);
      break;
    }

    case tok::kw___sptr:
    case tok::kw___uptr:
    case tok::kw___ptr64:
    case tok::kw___ptr32:
    case tok::kw___w64:
    case tok::kw___cdecl:
    case tok::kw___stdcall:
    case tok::kw___fastcall:
    case tok::kw___thiscall:
    case tok::kw___unaligned:
      ParseMicrosoftTypeAttributes(DS.getAttributes());
      continue;

    // Borland single token adornments.
    case tok::kw___pascal:
      ParseBorlandTypeAttributes(DS.getAttributes());
      continue;

    // OpenCL single token adornments.
    case tok::kw___kernel:
      ParseOpenCLAttributes(DS.getAttributes());
      continue;

    // storage-class-specifier
    case tok::kw_typedef:
      isInvalid = DS.SetStorageClassSpec(Actions, DeclSpec::SCS_typedef, Loc,
                                         PrevSpec, DiagID, Policy);
      break;
    case tok::kw_extern:
      if (DS.getThreadStorageClassSpec() == DeclSpec::TSCS___thread)
        Diag(Tok, diag::ext_thread_before) << "extern";
      isInvalid = DS.SetStorageClassSpec(Actions, DeclSpec::SCS_extern, Loc,
                                         PrevSpec, DiagID, Policy);
      break;
    case tok::kw___private_extern__:
      isInvalid = DS.SetStorageClassSpec(Actions, DeclSpec::SCS_private_extern,
                                         Loc, PrevSpec, DiagID, Policy);
      break;
    case tok::kw_static:
      if (DS.getThreadStorageClassSpec() == DeclSpec::TSCS___thread)
        Diag(Tok, diag::ext_thread_before) << "static";
      isInvalid = DS.SetStorageClassSpec(Actions, DeclSpec::SCS_static, Loc,
                                         PrevSpec, DiagID, Policy);
      break;
    case tok::kw_auto:
      if (getLangOpts().CPlusPlus11) {
        if (isKnownToBeTypeSpecifier(GetLookAheadToken(1))) {
          isInvalid = DS.SetStorageClassSpec(Actions, DeclSpec::SCS_auto, Loc,
                                             PrevSpec, DiagID, Policy);
          if (!isInvalid)
            Diag(Tok, diag::ext_auto_storage_class)
              << FixItHint::CreateRemoval(DS.getStorageClassSpecLoc());
        } else
          isInvalid = DS.SetTypeSpecType(DeclSpec::TST_auto, Loc, PrevSpec,
                                         DiagID, Policy);
      } else
        isInvalid = DS.SetStorageClassSpec(Actions, DeclSpec::SCS_auto, Loc,
                                           PrevSpec, DiagID, Policy);
      break;
    case tok::kw_register:
      isInvalid = DS.SetStorageClassSpec(Actions, DeclSpec::SCS_register, Loc,
                                         PrevSpec, DiagID, Policy);
      break;
    case tok::kw_mutable:
      isInvalid = DS.SetStorageClassSpec(Actions, DeclSpec::SCS_mutable, Loc,
                                         PrevSpec, DiagID, Policy);
      break;
    case tok::kw___thread:
      isInvalid = DS.SetStorageClassSpecThread(DeclSpec::TSCS___thread, Loc,
                                               PrevSpec, DiagID);
      break;
    case tok::kw_thread_local:
      isInvalid = DS.SetStorageClassSpecThread(DeclSpec::TSCS_thread_local, Loc,
                                               PrevSpec, DiagID);
      break;
    case tok::kw__Thread_local:
      isInvalid = DS.SetStorageClassSpecThread(DeclSpec::TSCS__Thread_local,
                                               Loc, PrevSpec, DiagID);
      break;

    // function-specifier
    case tok::kw_inline:
      isInvalid = DS.setFunctionSpecInline(Loc, PrevSpec, DiagID);
      break;
    case tok::kw_virtual:
      isInvalid = DS.setFunctionSpecVirtual(Loc, PrevSpec, DiagID);
      break;
    case tok::kw_explicit:
      isInvalid = DS.setFunctionSpecExplicit(Loc, PrevSpec, DiagID);
      break;
    case tok::kw__Noreturn:
      if (!getLangOpts().C11)
        Diag(Loc, diag::ext_c11_noreturn);
      isInvalid = DS.setFunctionSpecNoreturn(Loc, PrevSpec, DiagID);
      break;

    // alignment-specifier
    case tok::kw__Alignas:
      if (!getLangOpts().C11)
        Diag(Tok, diag::ext_c11_alignment) << Tok.getName();
      ParseAlignmentSpecifier(DS.getAttributes());
      continue;

    // friend
    case tok::kw_friend:
      if (DSContext == DSC_class)
        isInvalid = DS.SetFriendSpec(Loc, PrevSpec, DiagID);
      else {
        PrevSpec = ""; // not actually used by the diagnostic
        DiagID = diag::err_friend_invalid_in_context;
        isInvalid = true;
      }
      break;

    // Modules
    case tok::kw___module_private__:
      isInvalid = DS.setModulePrivateSpec(Loc, PrevSpec, DiagID);
      break;

    // constexpr
    case tok::kw_constexpr:
      isInvalid = DS.SetConstexprSpec(Loc, PrevSpec, DiagID);
      break;

    // type-specifier
    case tok::kw_short:
      isInvalid = DS.SetTypeSpecWidth(DeclSpec::TSW_short, Loc, PrevSpec,
                                      DiagID, Policy);
      break;
    case tok::kw_long:
      if (DS.getTypeSpecWidth() != DeclSpec::TSW_long)
        isInvalid = DS.SetTypeSpecWidth(DeclSpec::TSW_long, Loc, PrevSpec,
                                        DiagID, Policy);
      else
        isInvalid = DS.SetTypeSpecWidth(DeclSpec::TSW_longlong, Loc, PrevSpec,
                                        DiagID, Policy);
      break;
    case tok::kw___int64:
        isInvalid = DS.SetTypeSpecWidth(DeclSpec::TSW_longlong, Loc, PrevSpec,
                                        DiagID, Policy);
      break;
    case tok::kw_signed:
      isInvalid = DS.SetTypeSpecSign(DeclSpec::TSS_signed, Loc, PrevSpec,
                                     DiagID);
      break;
    case tok::kw_unsigned:
      isInvalid = DS.SetTypeSpecSign(DeclSpec::TSS_unsigned, Loc, PrevSpec,
                                     DiagID);
      break;
    case tok::kw__Complex:
      isInvalid = DS.SetTypeSpecComplex(DeclSpec::TSC_complex, Loc, PrevSpec,
                                        DiagID);
      break;
    case tok::kw__Imaginary:
      isInvalid = DS.SetTypeSpecComplex(DeclSpec::TSC_imaginary, Loc, PrevSpec,
                                        DiagID);
      break;
    case tok::kw_void:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_void, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_char:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_char, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_int:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_int, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw___int128:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_int128, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_half:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_half, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_float:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_float, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_double:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_double, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_wchar_t:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_wchar, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_char16_t:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_char16, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_char32_t:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_char32, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw_bool:
    case tok::kw__Bool:
      if (Tok.is(tok::kw_bool) &&
          DS.getTypeSpecType() != DeclSpec::TST_unspecified &&
          DS.getStorageClassSpec() == DeclSpec::SCS_typedef) {
        PrevSpec = ""; // Not used by the diagnostic.
        DiagID = diag::err_bool_redeclaration;
        // For better error recovery.
        Tok.setKind(tok::identifier);
        isInvalid = true;
      } else {
        isInvalid = DS.SetTypeSpecType(DeclSpec::TST_bool, Loc, PrevSpec,
                                       DiagID, Policy);
      }
      break;
    case tok::kw__Decimal32:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_decimal32, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw__Decimal64:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_decimal64, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw__Decimal128:
      isInvalid = DS.SetTypeSpecType(DeclSpec::TST_decimal128, Loc, PrevSpec,
                                     DiagID, Policy);
      break;
    case tok::kw___vector:
      isInvalid = DS.SetTypeAltiVecVector(true, Loc, PrevSpec, DiagID, Policy);
      break;
    case tok::kw___pixel:
      isInvalid = DS.SetTypeAltiVecPixel(true, Loc, PrevSpec, DiagID, Policy);
      break;
    case tok::kw___unknown_anytype:
      isInvalid = DS.SetTypeSpecType(TST_unknown_anytype, Loc,
                                     PrevSpec, DiagID, Policy);
      break;

    // class-specifier:
    case tok::kw_class:
    case tok::kw_struct:
    case tok::kw___interface:
    case tok::kw_union: {
      tok::TokenKind Kind = Tok.getKind();
      ConsumeToken();

      // These are attributes following class specifiers.
      // To produce better diagnostic, we parse them when
      // parsing class specifier.
      ParsedAttributesWithRange Attributes(AttrFactory);
      ParseClassSpecifier(Kind, Loc, DS, TemplateInfo, AS,
                          EnteringContext, DSContext, Attributes);

      // If there are attributes following class specifier,
      // take them over and handle them here.
      if (!Attributes.empty()) {
        AttrsLastTime = true;
        attrs.takeAllFrom(Attributes);
      }
      continue;
    }

    // enum-specifier:
    case tok::kw_enum:
      ConsumeToken();
      ParseEnumSpecifier(Loc, DS, TemplateInfo, AS, DSContext);
      continue;

    // cv-qualifier:
    case tok::kw_const:
      isInvalid = DS.SetTypeQual(DeclSpec::TQ_const, Loc, PrevSpec, DiagID,
                                 getLangOpts());
      break;
    case tok::kw_volatile:
      isInvalid = DS.SetTypeQual(DeclSpec::TQ_volatile, Loc, PrevSpec, DiagID,
                                 getLangOpts());
      break;
    case tok::kw_restrict:
      isInvalid = DS.SetTypeQual(DeclSpec::TQ_restrict, Loc, PrevSpec, DiagID,
                                 getLangOpts());
      break;

    // C++ typename-specifier:
    case tok::kw_typename:
      if (TryAnnotateTypeOrScopeToken()) {
        DS.SetTypeSpecError();
        goto DoneWithDeclSpec;
      }
      if (!Tok.is(tok::kw_typename))
        continue;
      break;

    // GNU typeof support.
    case tok::kw_typeof:
      ParseTypeofSpecifier(DS);
      continue;

    case tok::annot_decltype:
      ParseDecltypeSpecifier(DS);
      continue;

    case tok::kw___underlying_type:
      ParseUnderlyingTypeSpecifier(DS);
      continue;

    case tok::kw__Atomic:
      // C11 6.7.2.4/4:
      //   If the _Atomic keyword is immediately followed by a left parenthesis,
      //   it is interpreted as a type specifier (with a type name), not as a
      //   type qualifier.
      if (NextToken().is(tok::l_paren)) {
        ParseAtomicSpecifier(DS);
        continue;
      }
      isInvalid = DS.SetTypeQual(DeclSpec::TQ_atomic, Loc, PrevSpec, DiagID,
                                 getLangOpts());
      break;

    // OpenCL qualifiers:
    case tok::kw___private:
    case tok::kw___global:
    case tok::kw___local:
    case tok::kw___constant:
    case tok::kw___read_only:
    case tok::kw___write_only:
    case tok::kw___read_write:
      ParseOpenCLQualifiers(DS.getAttributes());
      break;

    case tok::less:
      // GCC ObjC supports types like "<SomeProtocol>" as a synonym for
      // "id<SomeProtocol>".  This is hopelessly old fashioned and dangerous,
      // but we support it.
      if (DS.hasTypeSpecifier() || !getLangOpts().ObjC1)
        goto DoneWithDeclSpec;

      if (!ParseObjCProtocolQualifiers(DS))
        Diag(Loc, diag::warn_objc_protocol_qualifier_missing_id)
          << FixItHint::CreateInsertion(Loc, "id")
          << SourceRange(Loc, DS.getSourceRange().getEnd());

      // Need to support trailing type qualifiers (e.g. "id<p> const").
      // If a type specifier follows, it will be diagnosed elsewhere.
      continue;
    }
    // If the specifier wasn't legal, issue a diagnostic.
    if (isInvalid) {
      assert(PrevSpec && "Method did not return previous specifier!");
      assert(DiagID);

      if (DiagID == diag::ext_duplicate_declspec)
        Diag(Tok, DiagID)
          << PrevSpec << FixItHint::CreateRemoval(Tok.getLocation());
      else
        Diag(Tok, DiagID) << PrevSpec;
    }

    DS.SetRangeEnd(Tok.getLocation());
    if (DiagID != diag::err_bool_redeclaration)
      ConsumeToken();

    AttrsLastTime = false;
  }
}

/// ParseStructDeclaration - Parse a struct declaration without the terminating
/// semicolon.
///
///       struct-declaration:
///         specifier-qualifier-list struct-declarator-list
/// [GNU]   __extension__ struct-declaration
/// [GNU]   specifier-qualifier-list
///       struct-declarator-list:
///         struct-declarator
///         struct-declarator-list ',' struct-declarator
/// [GNU]   struct-declarator-list ',' attributes[opt] struct-declarator
///       struct-declarator:
///         declarator
/// [GNU]   declarator attributes[opt]
///         declarator[opt] ':' constant-expression
/// [GNU]   declarator[opt] ':' constant-expression attributes[opt]
///
void Parser::
ParseStructDeclaration(ParsingDeclSpec &DS, FieldCallback &Fields) {

  if (Tok.is(tok::kw___extension__)) {
    // __extension__ silences extension warnings in the subexpression.
    ExtensionRAIIObject O(Diags);  // Use RAII to do this.
    ConsumeToken();
    return ParseStructDeclaration(DS, Fields);
  }

  // Parse the common specifier-qualifiers-list piece.
  ParseSpecifierQualifierList(DS);

  // If there are no declarators, this is a free-standing declaration
  // specifier. Let the actions module cope with it.
  if (Tok.is(tok::semi)) {
    Decl *TheDecl = Actions.ParsedFreeStandingDeclSpec(getCurScope(), AS_none,
                                                       DS);
    DS.complete(TheDecl);
    return;
  }

  // Read struct-declarators until we find the semicolon.
  bool FirstDeclarator = true;
  SourceLocation CommaLoc;
  while (1) {
    ParsingFieldDeclarator DeclaratorInfo(*this, DS);
    DeclaratorInfo.D.setCommaLoc(CommaLoc);

    // Attributes are only allowed here on successive declarators.
    if (!FirstDeclarator)
      MaybeParseGNUAttributes(DeclaratorInfo.D);

    /// struct-declarator: declarator
    /// struct-declarator: declarator[opt] ':' constant-expression
    if (Tok.isNot(tok::colon)) {
      // Don't parse FOO:BAR as if it were a typo for FOO::BAR.
      ColonProtectionRAIIObject X(*this);
      ParseDeclarator(DeclaratorInfo.D);
    }

    if (TryConsumeToken(tok::colon)) {
      ExprResult Res(ParseConstantExpression());
      if (Res.isInvalid())
        SkipUntil(tok::semi, StopBeforeMatch);
      else
        DeclaratorInfo.BitfieldSize = Res.release();
    }

    // If attributes exist after the declarator, parse them.
    MaybeParseGNUAttributes(DeclaratorInfo.D);

    // We're done with this declarator;  invoke the callback.
    Fields.invoke(DeclaratorInfo);

    // If we don't have a comma, it is either the end of the list (a ';')
    // or an error, bail out.
    if (!TryConsumeToken(tok::comma, CommaLoc))
      return;

    FirstDeclarator = false;
  }
}

/// ParseStructUnionBody
///       struct-contents:
///         struct-declaration-list
/// [EXT]   empty
/// [GNU]   "struct-declaration-list" without terminatoring ';'
///       struct-declaration-list:
///         struct-declaration
///         struct-declaration-list struct-declaration
/// [OBC]   '@' 'defs' '(' class-name ')'
///
void Parser::ParseStructUnionBody(SourceLocation RecordLoc,
                                  unsigned TagType, Decl *TagDecl) {
  PrettyDeclStackTraceEntry CrashInfo(Actions, TagDecl, RecordLoc,
                                      "parsing struct/union body");
  assert(!getLangOpts().CPlusPlus && "C++ declarations not supported");

  BalancedDelimiterTracker T(*this, tok::l_brace);
  if (T.consumeOpen())
    return;

  ParseScope StructScope(this, Scope::ClassScope|Scope::DeclScope);
  Actions.ActOnTagStartDefinition(getCurScope(), TagDecl);

  SmallVector<Decl *, 32> FieldDecls;

  // While we still have something to read, read the declarations in the struct.
  while (Tok.isNot(tok::r_brace) && !isEofOrEom()) {
    // Each iteration of this loop reads one struct-declaration.

    // Check for extraneous top-level semicolon.
    if (Tok.is(tok::semi)) {
      ConsumeExtraSemi(InsideStruct, TagType);
      continue;
    }

    // Parse _Static_assert declaration.
    if (Tok.is(tok::kw__Static_assert)) {
      SourceLocation DeclEnd;
      ParseStaticAssertDeclaration(DeclEnd);
      continue;
    }

    if (Tok.is(tok::annot_pragma_pack)) {
      HandlePragmaPack();
      continue;
    }

    if (Tok.is(tok::annot_pragma_align)) {
      HandlePragmaAlign();
      continue;
    }

    if (!Tok.is(tok::at)) {
      struct CFieldCallback : FieldCallback {
        Parser &P;
        Decl *TagDecl;
        SmallVectorImpl<Decl *> &FieldDecls;

        CFieldCallback(Parser &P, Decl *TagDecl,
                       SmallVectorImpl<Decl *> &FieldDecls) :
          P(P), TagDecl(TagDecl), FieldDecls(FieldDecls) {}

        void invoke(ParsingFieldDeclarator &FD) {
          // Install the declarator into the current TagDecl.
          Decl *Field = P.Actions.ActOnField(P.getCurScope(), TagDecl,
                              FD.D.getDeclSpec().getSourceRange().getBegin(),
                                                 FD.D, FD.BitfieldSize);
          FieldDecls.push_back(Field);
          FD.complete(Field);
        }
      } Callback(*this, TagDecl, FieldDecls);

      // Parse all the comma separated declarators.
      ParsingDeclSpec DS(*this);
      ParseStructDeclaration(DS, Callback);
    } else { // Handle @defs
      ConsumeToken();
      if (!Tok.isObjCAtKeyword(tok::objc_defs)) {
        Diag(Tok, diag::err_unexpected_at);
        SkipUntil(tok::semi);
        continue;
      }
      ConsumeToken();
      ExpectAndConsume(tok::l_paren);
      if (!Tok.is(tok::identifier)) {
        Diag(Tok, diag::err_expected) << tok::identifier;
        SkipUntil(tok::semi);
        continue;
      }
      SmallVector<Decl *, 16> Fields;
      Actions.ActOnDefs(getCurScope(), TagDecl, Tok.getLocation(),
                        Tok.getIdentifierInfo(), Fields);
      FieldDecls.insert(FieldDecls.end(), Fields.begin(), Fields.end());
      ConsumeToken();
      ExpectAndConsume(tok::r_paren);
    }

    if (TryConsumeToken(tok::semi))
      continue;

    if (Tok.is(tok::r_brace)) {
      ExpectAndConsume(tok::semi, diag::ext_expected_semi_decl_list);
      break;
    }

    ExpectAndConsume(tok::semi, diag::err_expected_semi_decl_list);
    // Skip to end of block or statement to avoid ext-warning on extra ';'.
    SkipUntil(tok::r_brace, StopAtSemi | StopBeforeMatch);
    // If we stopped at a ';', eat it.
    TryConsumeToken(tok::semi);
  }

  T.consumeClose();

  ParsedAttributes attrs(AttrFactory);
  // If attributes exist after struct contents, parse them.
  MaybeParseGNUAttributes(attrs);

  Actions.ActOnFields(getCurScope(),
                      RecordLoc, TagDecl, FieldDecls,
                      T.getOpenLocation(), T.getCloseLocation(),
                      attrs.getList());
  StructScope.Exit();
  Actions.ActOnTagFinishDefinition(getCurScope(), TagDecl,
                                   T.getCloseLocation());
}

/// ParseEnumSpecifier
///       enum-specifier: [C99 6.7.2.2]
///         'enum' identifier[opt] '{' enumerator-list '}'
///[C99/C++]'enum' identifier[opt] '{' enumerator-list ',' '}'
/// [GNU]   'enum' attributes[opt] identifier[opt] '{' enumerator-list ',' [opt]
///                                                 '}' attributes[opt]
/// [MS]    'enum' __declspec[opt] identifier[opt] '{' enumerator-list ',' [opt]
///                                                 '}'
///         'enum' identifier
/// [GNU]   'enum' attributes[opt] identifier
///
/// [C++11] enum-head '{' enumerator-list[opt] '}'
/// [C++11] enum-head '{' enumerator-list ','  '}'
///
///       enum-head: [C++11]
///         enum-key attribute-specifier-seq[opt] identifier[opt] enum-base[opt]
///         enum-key attribute-specifier-seq[opt] nested-name-specifier
///             identifier enum-base[opt]
///
///       enum-key: [C++11]
///         'enum'
///         'enum' 'class'
///         'enum' 'struct'
///
///       enum-base: [C++11]
///         ':' type-specifier-seq
///
/// [C++] elaborated-type-specifier:
/// [C++]   'enum' '::'[opt] nested-name-specifier[opt] identifier
///
void Parser::ParseEnumSpecifier(SourceLocation StartLoc, DeclSpec &DS,
                                const ParsedTemplateInfo &TemplateInfo,
                                AccessSpecifier AS, DeclSpecContext DSC) {
  // Parse the tag portion of this.
  if (Tok.is(tok::code_completion)) {
    // Code completion for an enum name.
    Actions.CodeCompleteTag(getCurScope(), DeclSpec::TST_enum);
    return cutOffParsing();
  }

  // If attributes exist after tag, parse them.
  ParsedAttributesWithRange attrs(AttrFactory);
  MaybeParseGNUAttributes(attrs);
  MaybeParseCXX11Attributes(attrs);

  // If declspecs exist after tag, parse them.
  while (Tok.is(tok::kw___declspec))
    ParseMicrosoftDeclSpec(attrs);

  SourceLocation ScopedEnumKWLoc;
  bool IsScopedUsingClassTag = false;

  // In C++11, recognize 'enum class' and 'enum struct'.
  if (Tok.is(tok::kw_class) || Tok.is(tok::kw_struct)) {
    Diag(Tok, getLangOpts().CPlusPlus11 ? diag::warn_cxx98_compat_scoped_enum
                                        : diag::ext_scoped_enum);
    IsScopedUsingClassTag = Tok.is(tok::kw_class);
    ScopedEnumKWLoc = ConsumeToken();

    // Attributes are not allowed between these keywords.  Diagnose,
    // but then just treat them like they appeared in the right place.
    ProhibitAttributes(attrs);

    // They are allowed afterwards, though.
    MaybeParseGNUAttributes(attrs);
    MaybeParseCXX11Attributes(attrs);
    while (Tok.is(tok::kw___declspec))
      ParseMicrosoftDeclSpec(attrs);
  }

  // C++11 [temp.explicit]p12:
  //   The usual access controls do not apply to names used to specify
  //   explicit instantiations.
  // We extend this to also cover explicit specializations.  Note that
  // we don't suppress if this turns out to be an elaborated type
  // specifier.
  bool shouldDelayDiagsInTag =
    (TemplateInfo.Kind == ParsedTemplateInfo::ExplicitInstantiation ||
     TemplateInfo.Kind == ParsedTemplateInfo::ExplicitSpecialization);
  SuppressAccessChecks diagsFromTag(*this, shouldDelayDiagsInTag);

  // Enum definitions should not be parsed in a trailing-return-type.
  bool AllowDeclaration = DSC != DSC_trailing;

  bool AllowFixedUnderlyingType = AllowDeclaration &&
    (getLangOpts().CPlusPlus11 || getLangOpts().MicrosoftExt ||
     getLangOpts().ObjC2);

  CXXScopeSpec &SS = DS.getTypeSpecScope();
  if (getLangOpts().CPlusPlus) {
    // "enum foo : bar;" is not a potential typo for "enum foo::bar;"
    // if a fixed underlying type is allowed.
    ColonProtectionRAIIObject X(*this, AllowFixedUnderlyingType);

    if (ParseOptionalCXXScopeSpecifier(SS, ParsedType(),
                                       /*EnteringContext=*/true))
      return;

    if (SS.isSet() && Tok.isNot(tok::identifier)) {
      Diag(Tok, diag::err_expected) << tok::identifier;
      if (Tok.isNot(tok::l_brace)) {
        // Has no name and is not a definition.
        // Skip the rest of this declarator, up until the comma or semicolon.
        SkipUntil(tok::comma, StopAtSemi);
        return;
      }
    }
  }

  // Must have either 'enum name' or 'enum {...}'.
  if (Tok.isNot(tok::identifier) && Tok.isNot(tok::l_brace) &&
      !(AllowFixedUnderlyingType && Tok.is(tok::colon))) {
    Diag(Tok, diag::err_expected_either) << tok::identifier << tok::l_brace;

    // Skip the rest of this declarator, up until the comma or semicolon.
    SkipUntil(tok::comma, StopAtSemi);
    return;
  }

  // If an identifier is present, consume and remember it.
  IdentifierInfo *Name = 0;
  SourceLocation NameLoc;
  if (Tok.is(tok::identifier)) {
    Name = Tok.getIdentifierInfo();
    NameLoc = ConsumeToken();
  }

  if (!Name && ScopedEnumKWLoc.isValid()) {
    // C++0x 7.2p2: The optional identifier shall not be omitted in the
    // declaration of a scoped enumeration.
    Diag(Tok, diag::err_scoped_enum_missing_identifier);
    ScopedEnumKWLoc = SourceLocation();
    IsScopedUsingClassTag = false;
  }

  // Okay, end the suppression area.  We'll decide whether to emit the
  // diagnostics in a second.
  if (shouldDelayDiagsInTag)
    diagsFromTag.done();

  TypeResult BaseType;

  // Parse the fixed underlying type.
  bool CanBeBitfield = getCurScope()->getFlags() & Scope::ClassScope;
  if (AllowFixedUnderlyingType && Tok.is(tok::colon)) {
    bool PossibleBitfield = false;
    if (CanBeBitfield) {
      // If we're in class scope, this can either be an enum declaration with
      // an underlying type, or a declaration of a bitfield member. We try to
      // use a simple disambiguation scheme first to catch the common cases
      // (integer literal, sizeof); if it's still ambiguous, we then consider
      // anything that's a simple-type-specifier followed by '(' as an
      // expression. This suffices because function types are not valid
      // underlying types anyway.
      EnterExpressionEvaluationContext Unevaluated(Actions,
                                                   Sema::ConstantEvaluated);
      TPResult TPR = isExpressionOrTypeSpecifierSimple(NextToken().getKind());
      // If the next token starts an expression, we know we're parsing a
      // bit-field. This is the common case.
      if (TPR == TPResult::True())
        PossibleBitfield = true;
      // If the next token starts a type-specifier-seq, it may be either a
      // a fixed underlying type or the start of a function-style cast in C++;
      // lookahead one more token to see if it's obvious that we have a
      // fixed underlying type.
      else if (TPR == TPResult::False() &&
               GetLookAheadToken(2).getKind() == tok::semi) {
        // Consume the ':'.
        ConsumeToken();
      } else {
        // We have the start of a type-specifier-seq, so we have to perform
        // tentative parsing to determine whether we have an expression or a
        // type.
        TentativeParsingAction TPA(*this);

        // Consume the ':'.
        ConsumeToken();

        // If we see a type specifier followed by an open-brace, we have an
        // ambiguity between an underlying type and a C++11 braced
        // function-style cast. Resolve this by always treating it as an
        // underlying type.
        // FIXME: The standard is not entirely clear on how to disambiguate in
        // this case.
        if ((getLangOpts().CPlusPlus &&
             isCXXDeclarationSpecifier(TPResult::True()) != TPResult::True()) ||
            (!getLangOpts().CPlusPlus && !isDeclarationSpecifier(true))) {
          // We'll parse this as a bitfield later.
          PossibleBitfield = true;
          TPA.Revert();
        } else {
          // We have a type-specifier-seq.
          TPA.Commit();
        }
      }
    } else {
      // Consume the ':'.
      ConsumeToken();
    }

    if (!PossibleBitfield) {
      SourceRange Range;
      BaseType = ParseTypeName(&Range);

      if (getLangOpts().CPlusPlus11) {
        Diag(StartLoc, diag::warn_cxx98_compat_enum_fixed_underlying_type);
      } else if (!getLangOpts().ObjC2) {
        if (getLangOpts().CPlusPlus)
          Diag(StartLoc, diag::ext_cxx11_enum_fixed_underlying_type) << Range;
        else
          Diag(StartLoc, diag::ext_c_enum_fixed_underlying_type) << Range;
      }
    }
  }

  // There are four options here.  If we have 'friend enum foo;' then this is a
  // friend declaration, and cannot have an accompanying definition. If we have
  // 'enum foo;', then this is a forward declaration.  If we have
  // 'enum foo {...' then this is a definition. Otherwise we have something
  // like 'enum foo xyz', a reference.
  //
  // This is needed to handle stuff like this right (C99 6.7.2.3p11):
  // enum foo {..};  void bar() { enum foo; }    <- new foo in bar.
  // enum foo {..};  void bar() { enum foo x; }  <- use of old foo.
  //
  Sema::TagUseKind TUK;
  if (!AllowDeclaration) {
    TUK = Sema::TUK_Reference;
  } else if (Tok.is(tok::l_brace)) {
    if (DS.isFriendSpecified()) {
      Diag(Tok.getLocation(), diag::err_friend_decl_defines_type)
        << SourceRange(DS.getFriendSpecLoc());
      ConsumeBrace();
      SkipUntil(tok::r_brace, StopAtSemi);
      TUK = Sema::TUK_Friend;
    } else {
      TUK = Sema::TUK_Definition;
    }
  } else if (!isTypeSpecifier(DSC) &&
             (Tok.is(tok::semi) ||
              (Tok.isAtStartOfLine() &&
               !isValidAfterTypeSpecifier(CanBeBitfield)))) {
    TUK = DS.isFriendSpecified() ? Sema::TUK_Friend : Sema::TUK_Declaration;
    if (Tok.isNot(tok::semi)) {
      // A semicolon was missing after this declaration. Diagnose and recover.
      ExpectAndConsume(tok::semi, diag::err_expected_after, "enum");
      PP.EnterToken(Tok);
      Tok.setKind(tok::semi);
    }
  } else {
    TUK = Sema::TUK_Reference;
  }

  // If this is an elaborated type specifier, and we delayed
  // diagnostics before, just merge them into the current pool.
  if (TUK == Sema::TUK_Reference && shouldDelayDiagsInTag) {
    diagsFromTag.redelay();
  }

  MultiTemplateParamsArg TParams;
  if (TemplateInfo.Kind != ParsedTemplateInfo::NonTemplate &&
      TUK != Sema::TUK_Reference) {
    if (!getLangOpts().CPlusPlus11 || !SS.isSet()) {
      // Skip the rest of this declarator, up until the comma or semicolon.
      Diag(Tok, diag::err_enum_template);
      SkipUntil(tok::comma, StopAtSemi);
      return;
    }

    if (TemplateInfo.Kind == ParsedTemplateInfo::ExplicitInstantiation) {
      // Enumerations can't be explicitly instantiated.
      DS.SetTypeSpecError();
      Diag(StartLoc, diag::err_explicit_instantiation_enum);
      return;
    }

    assert(TemplateInfo.TemplateParams && "no template parameters");
    TParams = MultiTemplateParamsArg(TemplateInfo.TemplateParams->data(),
                                     TemplateInfo.TemplateParams->size());
  }

  if (TUK == Sema::TUK_Reference)
    ProhibitAttributes(attrs);

  if (!Name && TUK != Sema::TUK_Definition) {
    Diag(Tok, diag::err_enumerator_unnamed_no_def);

    // Skip the rest of this declarator, up until the comma or semicolon.
    SkipUntil(tok::comma, StopAtSemi);
    return;
  }

  bool Owned = false;
  bool IsDependent = false;
  const char *PrevSpec = 0;
  unsigned DiagID;
  Decl *TagDecl = Actions.ActOnTag(getCurScope(), DeclSpec::TST_enum, TUK,
                                   StartLoc, SS, Name, NameLoc, attrs.getList(),
                                   AS, DS.getModulePrivateSpecLoc(), TParams,
                                   Owned, IsDependent, ScopedEnumKWLoc,
                                   IsScopedUsingClassTag, BaseType,
                                   DSC == DSC_type_specifier);

  if (IsDependent) {
    // This enum has a dependent nested-name-specifier. Handle it as a
    // dependent tag.
    if (!Name) {
      DS.SetTypeSpecError();
      Diag(Tok, diag::err_expected_type_name_after_typename);
      return;
    }

    TypeResult Type = Actions.ActOnDependentTag(getCurScope(), DeclSpec::TST_enum,
                                                TUK, SS, Name, StartLoc,
                                                NameLoc);
    if (Type.isInvalid()) {
      DS.SetTypeSpecError();
      return;
    }

    if (DS.SetTypeSpecType(DeclSpec::TST_typename, StartLoc,
                           NameLoc.isValid() ? NameLoc : StartLoc,
                           PrevSpec, DiagID, Type.get(),
                           Actions.getASTContext().getPrintingPolicy()))
      Diag(StartLoc, DiagID) << PrevSpec;

    return;
  }

  if (!TagDecl) {
    // The action failed to produce an enumeration tag. If this is a
    // definition, consume the entire definition.
    if (Tok.is(tok::l_brace) && TUK != Sema::TUK_Reference) {
      ConsumeBrace();
      SkipUntil(tok::r_brace, StopAtSemi);
    }

    DS.SetTypeSpecError();
    return;
  }

  if (Tok.is(tok::l_brace) && TUK != Sema::TUK_Reference)
    ParseEnumBody(StartLoc, TagDecl);

  if (DS.SetTypeSpecType(DeclSpec::TST_enum, StartLoc,
                         NameLoc.isValid() ? NameLoc : StartLoc,
                         PrevSpec, DiagID, TagDecl, Owned,
                         Actions.getASTContext().getPrintingPolicy()))
    Diag(StartLoc, DiagID) << PrevSpec;
}

/// ParseEnumBody - Parse a {} enclosed enumerator-list.
///       enumerator-list:
///         enumerator
///         enumerator-list ',' enumerator
///       enumerator:
///         enumeration-constant
///         enumeration-constant '=' constant-expression
///       enumeration-constant:
///         identifier
///
void Parser::ParseEnumBody(SourceLocation StartLoc, Decl *EnumDecl) {
  // Enter the scope of the enum body and start the definition.
  ParseScope EnumScope(this, Scope::DeclScope);
  Actions.ActOnTagStartDefinition(getCurScope(), EnumDecl);

  BalancedDelimiterTracker T(*this, tok::l_brace);
  T.consumeOpen();

  // C does not allow an empty enumerator-list, C++ does [dcl.enum].
  if (Tok.is(tok::r_brace) && !getLangOpts().CPlusPlus)
    Diag(Tok, diag::error_empty_enum);

  SmallVector<Decl *, 32> EnumConstantDecls;

  Decl *LastEnumConstDecl = 0;

  // Parse the enumerator-list.
  while (Tok.isNot(tok::r_brace)) {
    // Parse enumerator. If failed, try skipping till the start of the next
    // enumerator definition.
    if (Tok.isNot(tok::identifier)) {
      Diag(Tok.getLocation(), diag::err_expected) << tok::identifier;
      if (SkipUntil(tok::comma, tok::r_brace, StopBeforeMatch) &&
          TryConsumeToken(tok::comma))
        continue;
      break;
    }
    IdentifierInfo *Ident = Tok.getIdentifierInfo();
    SourceLocation IdentLoc = ConsumeToken();

    // If attributes exist after the enumerator, parse them.
    ParsedAttributesWithRange attrs(AttrFactory);
    MaybeParseGNUAttributes(attrs);
    MaybeParseCXX11Attributes(attrs);
    ProhibitAttributes(attrs);

    SourceLocation EqualLoc;
    ExprResult AssignedVal;
    ParsingDeclRAIIObject PD(*this, ParsingDeclRAIIObject::NoParent);

    if (TryConsumeToken(tok::equal, EqualLoc)) {
      AssignedVal = ParseConstantExpression();
      if (AssignedVal.isInvalid())
        SkipUntil(tok::comma, tok::r_brace, StopBeforeMatch);
    }

    // Install the enumerator constant into EnumDecl.
    Decl *EnumConstDecl = Actions.ActOnEnumConstant(getCurScope(), EnumDecl,
                                                    LastEnumConstDecl,
                                                    IdentLoc, Ident,
                                                    attrs.getList(), EqualLoc,
                                                    AssignedVal.release());
    PD.complete(EnumConstDecl);

    EnumConstantDecls.push_back(EnumConstDecl);
    LastEnumConstDecl = EnumConstDecl;

    if (Tok.is(tok::identifier)) {
      // We're missing a comma between enumerators.
      SourceLocation Loc = PP.getLocForEndOfToken(PrevTokLocation);
      Diag(Loc, diag::err_enumerator_list_missing_comma)
        << FixItHint::CreateInsertion(Loc, ", ");
      continue;
    }

    // Emumerator definition must be finished, only comma or r_brace are
    // allowed here.
    SourceLocation CommaLoc;
    if (Tok.isNot(tok::r_brace) && !TryConsumeToken(tok::comma, CommaLoc)) {
      if (EqualLoc.isValid())
        Diag(Tok.getLocation(), diag::err_expected_either) << tok::r_brace
                                                           << tok::comma;
      else
        Diag(Tok.getLocation(), diag::err_expected_end_of_enumerator);
      if (SkipUntil(tok::comma, tok::r_brace, StopBeforeMatch)) {
        if (TryConsumeToken(tok::comma, CommaLoc))
          continue;
      } else {
        break;
      }
    }

    // If comma is followed by r_brace, emit appropriate warning.
    if (Tok.is(tok::r_brace) && CommaLoc.isValid()) {
      if (!getLangOpts().C99 && !getLangOpts().CPlusPlus11)
        Diag(CommaLoc, getLangOpts().CPlusPlus ?
               diag::ext_enumerator_list_comma_cxx :
               diag::ext_enumerator_list_comma_c)
          << FixItHint::CreateRemoval(CommaLoc);
      else if (getLangOpts().CPlusPlus11)
        Diag(CommaLoc, diag::warn_cxx98_compat_enumerator_list_comma)
          << FixItHint::CreateRemoval(CommaLoc);
      break;
    }
  }

  // Eat the }.
  T.consumeClose();

  // If attributes exist after the identifier list, parse them.
  ParsedAttributes attrs(AttrFactory);
  MaybeParseGNUAttributes(attrs);

  Actions.ActOnEnumBody(StartLoc, T.getOpenLocation(), T.getCloseLocation(),
                        EnumDecl, EnumConstantDecls,
                        getCurScope(),
                        attrs.getList());

  EnumScope.Exit();
  Actions.ActOnTagFinishDefinition(getCurScope(), EnumDecl,
                                   T.getCloseLocation());

  // The next token must be valid after an enum definition. If not, a ';'
  // was probably forgotten.
  bool CanBeBitfield = getCurScope()->getFlags() & Scope::ClassScope;
  if (!isValidAfterTypeSpecifier(CanBeBitfield)) {
    ExpectAndConsume(tok::semi, diag::err_expected_after, "enum");
    // Push this token back into the preprocessor and change our current token
    // to ';' so that the rest of the code recovers as though there were an
    // ';' after the definition.
    PP.EnterToken(Tok);
    Tok.setKind(tok::semi);
  }
}

/// isTypeSpecifierQualifier - Return true if the current token could be the
/// start of a type-qualifier-list.
bool Parser::isTypeQualifier() const {
  switch (Tok.getKind()) {
  default: return false;
  // type-qualifier
  case tok::kw_const:
  case tok::kw_volatile:
  case tok::kw_restrict:
  case tok::kw___private:
  case tok::kw___local:
  case tok::kw___global:
  case tok::kw___constant:
  case tok::kw___read_only:
  case tok::kw___read_write:
  case tok::kw___write_only:
    return true;
  }
}

/// isKnownToBeTypeSpecifier - Return true if we know that the specified token
/// is definitely a type-specifier.  Return false if it isn't part of a type
/// specifier or if we're not sure.
bool Parser::isKnownToBeTypeSpecifier(const Token &Tok) const {
  switch (Tok.getKind()) {
  default: return false;
    // type-specifiers
  case tok::kw_short:
  case tok::kw_long:
  case tok::kw___int64:
  case tok::kw___int128:
  case tok::kw_signed:
  case tok::kw_unsigned:
  case tok::kw__Complex:
  case tok::kw__Imaginary:
  case tok::kw_void:
  case tok::kw_char:
  case tok::kw_wchar_t:
  case tok::kw_char16_t:
  case tok::kw_char32_t:
  case tok::kw_int:
  case tok::kw_half:
  case tok::kw_float:
  case tok::kw_double:
  case tok::kw_bool:
  case tok::kw__Bool:
  case tok::kw__Decimal32:
  case tok::kw__Decimal64:
  case tok::kw__Decimal128:
  case tok::kw___vector:

    // struct-or-union-specifier (C99) or class-specifier (C++)
  case tok::kw_class:
  case tok::kw_struct:
  case tok::kw___interface:
  case tok::kw_union:
    // enum-specifier
  case tok::kw_enum:

    // typedef-name
  case tok::annot_typename:
    return true;
  }
}

/// isTypeSpecifierQualifier - Return true if the current token could be the
/// start of a specifier-qualifier-list.
bool Parser::isTypeSpecifierQualifier() {
  switch (Tok.getKind()) {
  default: return false;

  case tok::identifier:   // foo::bar
    if (TryAltiVecVectorToken())
      return true;
    // Fall through.
  case tok::kw_typename:  // typename T::type
    // Annotate typenames and C++ scope specifiers.  If we get one, just
    // recurse to handle whatever we get.
    if (TryAnnotateTypeOrScopeToken())
      return true;
    if (Tok.is(tok::identifier))
      return false;
    return isTypeSpecifierQualifier();

  case tok::coloncolon:   // ::foo::bar
    if (NextToken().is(tok::kw_new) ||    // ::new
        NextToken().is(tok::kw_delete))   // ::delete
      return false;

    if (TryAnnotateTypeOrScopeToken())
      return true;
    return isTypeSpecifierQualifier();

    // GNU attributes support.
  case tok::kw___attribute:
    // GNU typeof support.
  case tok::kw_typeof:

    // type-specifiers
  case tok::kw_short:
  case tok::kw_long:
  case tok::kw___int64:
  case tok::kw___int128:
  case tok::kw_signed:
  case tok::kw_unsigned:
  case tok::kw__Complex:
  case tok::kw__Imaginary:
  case tok::kw_void:
  case tok::kw_char:
  case tok::kw_wchar_t:
  case tok::kw_char16_t:
  case tok::kw_char32_t:
  case tok::kw_int:
  case tok::kw_half:
  case tok::kw_float:
  case tok::kw_double:
  case tok::kw_bool:
  case tok::kw__Bool:
  case tok::kw__Decimal32:
  case tok::kw__Decimal64:
  case tok::kw__Decimal128:
  case tok::kw___vector:

    // struct-or-union-specifier (C99) or class-specifier (C++)
  case tok::kw_class:
  case tok::kw_struct:
  case tok::kw___interface:
  case tok::kw_union:
    // enum-specifier
  case tok::kw_enum:

    // type-qualifier
  case tok::kw_const:
  case tok::kw_volatile:
  case tok::kw_restrict:

    // Debugger support.
  case tok::kw___unknown_anytype:

    // typedef-name
  case tok::annot_typename:
    return true;

    // GNU ObjC bizarre protocol extension: <proto1,proto2> with implicit 'id'.
  case tok::less:
    return getLangOpts().ObjC1;

  case tok::kw___cdecl:
  case tok::kw___stdcall:
  case tok::kw___fastcall:
  case tok::kw___thiscall:
  case tok::kw___w64:
  case tok::kw___ptr64:
  case tok::kw___ptr32:
  case tok::kw___pascal:
  case tok::kw___unaligned:

  case tok::kw___private:
  case tok::kw___local:
  case tok::kw___global:
  case tok::kw___constant:
  case tok::kw___read_only:
  case tok::kw___read_write:
  case tok::kw___write_only:

    return true;

  // C11 _Atomic
  case tok::kw__Atomic:
    return true;
  }
}

/// isDeclarationSpecifier() - Return true if the current token is part of a
/// declaration specifier.
///
/// \param DisambiguatingWithExpression True to indicate that the purpose of
/// this check is to disambiguate between an expression and a declaration.
bool Parser::isDeclarationSpecifier(bool DisambiguatingWithExpression) {
  switch (Tok.getKind()) {
  default: return false;

  case tok::identifier:   // foo::bar
    // Unfortunate hack to support "Class.factoryMethod" notation.
    if (getLangOpts().ObjC1 && NextToken().is(tok::period))
      return false;
    if (TryAltiVecVectorToken())
      return true;
    // Fall through.
  case tok::kw_decltype: // decltype(T())::type
  case tok::kw_typename: // typename T::type
    // Annotate typenames and C++ scope specifiers.  If we get one, just
    // recurse to handle whatever we get.
    if (TryAnnotateTypeOrScopeToken())
      return true;
    if (Tok.is(tok::identifier))
      return false;

    // If we're in Objective-C and we have an Objective-C class type followed
    // by an identifier and then either ':' or ']', in a place where an
    // expression is permitted, then this is probably a class message send
    // missing the initial '['. In this case, we won't consider this to be
    // the start of a declaration.
    if (DisambiguatingWithExpression &&
        isStartOfObjCClassMessageMissingOpenBracket())
      return false;

    return isDeclarationSpecifier();

  case tok::coloncolon:   // ::foo::bar
    if (NextToken().is(tok::kw_new) ||    // ::new
        NextToken().is(tok::kw_delete))   // ::delete
      return false;

    // Annotate typenames and C++ scope specifiers.  If we get one, just
    // recurse to handle whatever we get.
    if (TryAnnotateTypeOrScopeToken())
      return true;
    return isDeclarationSpecifier();

    // storage-class-specifier
  case tok::kw_typedef:
  case tok::kw_extern:
  case tok::kw___private_extern__:
  case tok::kw_static:
  case tok::kw_auto:
  case tok::kw_register:
  case tok::kw___thread:
  case tok::kw_thread_local:
  case tok::kw__Thread_local:

    // Modules
  case tok::kw___module_private__:

    // Debugger support
  case tok::kw___unknown_anytype:

    // type-specifiers
  case tok::kw_short:
  case tok::kw_long:
  case tok::kw___int64:
  case tok::kw___int128:
  case tok::kw_signed:
  case tok::kw_unsigned:
  case tok::kw__Complex:
  case tok::kw__Imaginary:
  case tok::kw_void:
  case tok::kw_char:
  case tok::kw_wchar_t:
  case tok::kw_char16_t:
  case tok::kw_char32_t:

  case tok::kw_int:
  case tok::kw_half:
  case tok::kw_float:
  case tok::kw_double:
  case tok::kw_bool:
  case tok::kw__Bool:
  case tok::kw__Decimal32:
  case tok::kw__Decimal64:
  case tok::kw__Decimal128:
  case tok::kw___vector:

    // struct-or-union-specifier (C99) or class-specifier (C++)
  case tok::kw_class:
  case tok::kw_struct:
  case tok::kw_union:
  case tok::kw___interface:
    // enum-specifier
  case tok::kw_enum:

    // type-qualifier
  case tok::kw_const:
  case tok::kw_volatile:
  case tok::kw_restrict:

    // function-specifier
  case tok::kw_inline:
  case tok::kw_virtual:
  case tok::kw_explicit:
  case tok::kw__Noreturn:

    // alignment-specifier
  case tok::kw__Alignas:

    // friend keyword.
  case tok::kw_friend:

    // static_assert-declaration
  case tok::kw__Static_assert:

    // GNU typeof support.
  case tok::kw_typeof:

    // GNU attributes.
  case tok::kw___attribute:

    // C++11 decltype and constexpr.
  case tok::annot_decltype:
  case tok::kw_constexpr:

    // C11 _Atomic
  case tok::kw__Atomic:
    return true;

    // GNU ObjC bizarre protocol extension: <proto1,proto2> with implicit 'id'.
  case tok::less:
    return getLangOpts().ObjC1;

    // typedef-name
  case tok::annot_typename:
    return !DisambiguatingWithExpression ||
           !isStartOfObjCClassMessageMissingOpenBracket();

  case tok::kw___declspec:
  case tok::kw___cdecl:
  case tok::kw___stdcall:
  case tok::kw___fastcall:
  case tok::kw___thiscall:
  case tok::kw___w64:
  case tok::kw___sptr:
  case tok::kw___uptr:
  case tok::kw___ptr64:
  case tok::kw___ptr32:
  case tok::kw___forceinline:
  case tok::kw___pascal:
  case tok::kw___unaligned:

  case tok::kw___private:
  case tok::kw___local:
  case tok::kw___global:
  case tok::kw___constant:
  case tok::kw___read_only:
  case tok::kw___read_write:
  case tok::kw___write_only:

    return true;
  }
}

bool Parser::isConstructorDeclarator(bool IsUnqualified) {
  TentativeParsingAction TPA(*this);

  // Parse the C++ scope specifier.
  CXXScopeSpec SS;
  if (ParseOptionalCXXScopeSpecifier(SS, ParsedType(),
                                     /*EnteringContext=*/true)) {
    TPA.Revert();
    return false;
  }

  // Parse the constructor name.
  if (Tok.is(tok::identifier) || Tok.is(tok::annot_template_id)) {
    // We already know that we have a constructor name; just consume
    // the token.
    ConsumeToken();
  } else {
    TPA.Revert();
    return false;
  }

  // Current class name must be followed by a left parenthesis.
  if (Tok.isNot(tok::l_paren)) {
    TPA.Revert();
    return false;
  }
  ConsumeParen();

  // A right parenthesis, or ellipsis followed by a right parenthesis signals
  // that we have a constructor.
  if (Tok.is(tok::r_paren) ||
      (Tok.is(tok::ellipsis) && NextToken().is(tok::r_paren))) {
    TPA.Revert();
    return true;
  }

  // A C++11 attribute here signals that we have a constructor, and is an
  // attribute on the first constructor parameter.
  if (getLangOpts().CPlusPlus11 &&
      isCXX11AttributeSpecifier(/*Disambiguate*/ false,
                                /*OuterMightBeMessageSend*/ true)) {
    TPA.Revert();
    return true;
  }

  // If we need to, enter the specified scope.
  DeclaratorScopeObj DeclScopeObj(*this, SS);
  if (SS.isSet() && Actions.ShouldEnterDeclaratorScope(getCurScope(), SS))
    DeclScopeObj.EnterDeclaratorScope();

  // Optionally skip Microsoft attributes.
  ParsedAttributes Attrs(AttrFactory);
  MaybeParseMicrosoftAttributes(Attrs);

  // Check whether the next token(s) are part of a declaration
  // specifier, in which case we have the start of a parameter and,
  // therefore, we know that this is a constructor.
  bool IsConstructor = false;
  if (isDeclarationSpecifier())
    IsConstructor = true;
  else if (Tok.is(tok::identifier) ||
           (Tok.is(tok::annot_cxxscope) && NextToken().is(tok::identifier))) {
    // We've seen "C ( X" or "C ( X::Y", but "X" / "X::Y" is not a type.
    // This might be a parenthesized member name, but is more likely to
    // be a constructor declaration with an invalid argument type. Keep
    // looking.
    if (Tok.is(tok::annot_cxxscope))
      ConsumeToken();
    ConsumeToken();

    // If this is not a constructor, we must be parsing a declarator,
    // which must have one of the following syntactic forms (see the
    // grammar extract at the start of ParseDirectDeclarator):
    switch (Tok.getKind()) {
    case tok::l_paren:
      // C(X   (   int));
    case tok::l_square:
      // C(X   [   5]);
      // C(X   [   [attribute]]);
    case tok::coloncolon:
      // C(X   ::   Y);
      // C(X   ::   *p);
      // Assume this isn't a constructor, rather than assuming it's a
      // constructor with an unnamed parameter of an ill-formed type.
      break;

    case tok::r_paren:
      // C(X   )
      if (NextToken().is(tok::colon) || NextToken().is(tok::kw_try)) {
        // Assume these were meant to be constructors:
        //   C(X)   :    (the name of a bit-field cannot be parenthesized).
        //   C(X)   try  (this is otherwise ill-formed).
        IsConstructor = true;
      }
      if (NextToken().is(tok::semi) || NextToken().is(tok::l_brace)) {
        // If we have a constructor name within the class definition,
        // assume these were meant to be constructors:
        //   C(X)   {
        //   C(X)   ;
        // ... because otherwise we would be declaring a non-static data
        // member that is ill-formed because it's of the same type as its
        // surrounding class.
        //
        // FIXME: We can actually do this whether or not the name is qualified,
        // because if it is qualified in this context it must be being used as
        // a constructor name. However, we do not implement that rule correctly
        // currently, so we're somewhat conservative here.
        IsConstructor = IsUnqualified;
      }
      break;

    default:
      IsConstructor = true;
      break;
    }
  }

  TPA.Revert();
  return IsConstructor;
}

/// ParseTypeQualifierListOpt
///          type-qualifier-list: [C99 6.7.5]
///            type-qualifier
/// [vendor]   attributes
///              [ only if VendorAttributesAllowed=true ]
///            type-qualifier-list type-qualifier
/// [vendor]   type-qualifier-list attributes
///              [ only if VendorAttributesAllowed=true ]
/// [C++0x]    attribute-specifier[opt] is allowed before cv-qualifier-seq
///              [ only if CXX11AttributesAllowed=true ]
/// Note: vendor can be GNU, MS, etc.
///
void Parser::ParseTypeQualifierListOpt(DeclSpec &DS,
                                       bool VendorAttributesAllowed,
                                       bool CXX11AttributesAllowed,
                                       bool AtomicAllowed,
                                       bool IdentifierRequired) {
  if (getLangOpts().CPlusPlus11 && CXX11AttributesAllowed &&
      isCXX11AttributeSpecifier()) {
    ParsedAttributesWithRange attrs(AttrFactory);
    ParseCXX11Attributes(attrs);
    DS.takeAttributesFrom(attrs);
  }

  SourceLocation EndLoc;

  while (1) {
    bool isInvalid = false;
    const char *PrevSpec = 0;
    unsigned DiagID = 0;
    SourceLocation Loc = Tok.getLocation();

    switch (Tok.getKind()) {
    case tok::code_completion:
      Actions.CodeCompleteTypeQualifiers(DS);
      return cutOffParsing();

    case tok::kw_const:
      isInvalid = DS.SetTypeQual(DeclSpec::TQ_const   , Loc, PrevSpec, DiagID,
                                 getLangOpts());
      break;
    case tok::kw_volatile:
      isInvalid = DS.SetTypeQual(DeclSpec::TQ_volatile, Loc, PrevSpec, DiagID,
                                 getLangOpts());
      break;
    case tok::kw_restrict:
      isInvalid = DS.SetTypeQual(DeclSpec::TQ_restrict, Loc, PrevSpec, DiagID,
                                 getLangOpts());
      break;
    case tok::kw__Atomic:
      if (!AtomicAllowed)
        goto DoneWithTypeQuals;
      isInvalid = DS.SetTypeQual(DeclSpec::TQ_atomic, Loc, PrevSpec, DiagID,
                                 getLangOpts());
      break;

    // OpenCL qualifiers:
    case tok::kw___private:
    case tok::kw___global:
    case tok::kw___local:
    case tok::kw___constant:
    case tok::kw___read_only:
    case tok::kw___write_only:
    case tok::kw___read_write:
      ParseOpenCLQualifiers(DS.getAttributes());
      break;

    case tok::kw___uptr:
      // GNU libc headers in C mode use '__uptr' as an identifer which conflicts
      // with the MS modifier keyword.
      if (VendorAttributesAllowed && !getLangOpts().CPlusPlus &&
          IdentifierRequired && DS.isEmpty() && NextToken().is(tok::semi)) {
        if (TryKeywordIdentFallback(false))
          continue;
      }
    case tok::kw___sptr:
    case tok::kw___w64:
    case tok::kw___ptr64:
    case tok::kw___ptr32:
    case tok::kw___cdecl:
    case tok::kw___stdcall:
    case tok::kw___fastcall:
    case tok::kw___thiscall:
    case tok::kw___unaligned:
      if (VendorAttributesAllowed) {
        ParseMicrosoftTypeAttributes(DS.getAttributes());
        continue;
      }
      goto DoneWithTypeQuals;
    case tok::kw___pascal:
      if (VendorAttributesAllowed) {
        ParseBorlandTypeAttributes(DS.getAttributes());
        continue;
      }
      goto DoneWithTypeQuals;
    case tok::kw___attribute:
      if (VendorAttributesAllowed) {
        ParseGNUAttributes(DS.getAttributes());
        continue; // do *not* consume the next token!
      }
      // otherwise, FALL THROUGH!
    default:
      DoneWithTypeQuals:
      // If this is not a type-qualifier token, we're done reading type
      // qualifiers.  First verify that DeclSpec's are consistent.
      DS.Finish(Diags, PP, Actions.getASTContext().getPrintingPolicy());
      if (EndLoc.isValid())
        DS.SetRangeEnd(EndLoc);
      return;
    }

    // If the specifier combination wasn't legal, issue a diagnostic.
    if (isInvalid) {
      assert(PrevSpec && "Method did not return previous specifier!");
      Diag(Tok, DiagID) << PrevSpec;
    }
    EndLoc = ConsumeToken();
  }
}


/// ParseDeclarator - Parse and verify a newly-initialized declarator.
///
void Parser::ParseDeclarator(Declarator &D) {
  /// This implements the 'declarator' production in the C grammar, then checks
  /// for well-formedness and issues diagnostics.
  ParseDeclaratorInternal(D, &Parser::ParseDirectDeclarator);
}

static bool isPtrOperatorToken(tok::TokenKind Kind, const LangOptions &Lang) {
  if (Kind == tok::star || Kind == tok::caret)
    return true;

  // We parse rvalue refs in C++03, because otherwise the errors are scary.
  if (!Lang.CPlusPlus)
    return false;

  return Kind == tok::amp || Kind == tok::ampamp;
}

/// ParseDeclaratorInternal - Parse a C or C++ declarator. The direct-declarator
/// is parsed by the function passed to it. Pass null, and the direct-declarator
/// isn't parsed at all, making this function effectively parse the C++
/// ptr-operator production.
///
/// If the grammar of this construct is extended, matching changes must also be
/// made to TryParseDeclarator and MightBeDeclarator, and possibly to
/// isConstructorDeclarator.
///
///       declarator: [C99 6.7.5] [C++ 8p4, dcl.decl]
/// [C]     pointer[opt] direct-declarator
/// [C++]   direct-declarator
/// [C++]   ptr-operator declarator
///
///       pointer: [C99 6.7.5]
///         '*' type-qualifier-list[opt]
///         '*' type-qualifier-list[opt] pointer
///
///       ptr-operator:
///         '*' cv-qualifier-seq[opt]
///         '&'
/// [C++0x] '&&'
/// [GNU]   '&' restrict[opt] attributes[opt]
/// [GNU?]  '&&' restrict[opt] attributes[opt]
///         '::'[opt] nested-name-specifier '*' cv-qualifier-seq[opt]
void Parser::ParseDeclaratorInternal(Declarator &D,
                                     DirectDeclParseFunction DirectDeclParser) {
  if (Diags.hasAllExtensionsSilenced())
    D.setExtension();

  // C++ member pointers start with a '::' or a nested-name.
  // Member pointers get special handling, since there's no place for the
  // scope spec in the generic path below.
  if (getLangOpts().CPlusPlus &&
      (Tok.is(tok::coloncolon) || Tok.is(tok::identifier) ||
       Tok.is(tok::annot_cxxscope))) {
    bool EnteringContext = D.getContext() == Declarator::FileContext ||
                           D.getContext() == Declarator::MemberContext;
    CXXScopeSpec SS;
    ParseOptionalCXXScopeSpecifier(SS, ParsedType(), EnteringContext);

    if (SS.isNotEmpty()) {
      if (Tok.isNot(tok::star)) {
        // The scope spec really belongs to the direct-declarator.
        if (D.mayHaveIdentifier())
          D.getCXXScopeSpec() = SS;
        else
          AnnotateScopeToken(SS, true);

        if (DirectDeclParser)
          (this->*DirectDeclParser)(D);
        return;
      }

      SourceLocation Loc = ConsumeToken();
      D.SetRangeEnd(Loc);
      DeclSpec DS(AttrFactory);
      ParseTypeQualifierListOpt(DS);
      D.ExtendWithDeclSpec(DS);

      // Recurse to parse whatever is left.
      ParseDeclaratorInternal(D, DirectDeclParser);

      // Sema will have to catch (syntactically invalid) pointers into global
      // scope. It has to catch pointers into namespace scope anyway.
      D.AddTypeInfo(DeclaratorChunk::getMemberPointer(SS,DS.getTypeQualifiers(),
                                                      Loc),
                    DS.getAttributes(),
                    /* Don't replace range end. */SourceLocation());
      return;
    }
  }

  tok::TokenKind Kind = Tok.getKind();
  // Not a pointer, C++ reference, or block.
  if (!isPtrOperatorToken(Kind, getLangOpts())) {
    if (DirectDeclParser)
      (this->*DirectDeclParser)(D);
    return;
  }

  // Otherwise, '*' -> pointer, '^' -> block, '&' -> lvalue reference,
  // '&&' -> rvalue reference
  SourceLocation Loc = ConsumeToken();  // Eat the *, ^, & or &&.
  D.SetRangeEnd(Loc);

  if (Kind == tok::star || Kind == tok::caret) {
    // Is a pointer.
    DeclSpec DS(AttrFactory);

    // FIXME: GNU attributes are not allowed here in a new-type-id.
    ParseTypeQualifierListOpt(DS, true, true, true, !D.mayOmitIdentifier());
    D.ExtendWithDeclSpec(DS);

    // Recursively parse the declarator.
    ParseDeclaratorInternal(D, DirectDeclParser);
    if (Kind == tok::star)
      // Remember that we parsed a pointer type, and remember the type-quals.
      D.AddTypeInfo(DeclaratorChunk::getPointer(DS.getTypeQualifiers(), Loc,
                                                DS.getConstSpecLoc(),
                                                DS.getVolatileSpecLoc(),
                                                DS.getRestrictSpecLoc()),
                    DS.getAttributes(),
                    SourceLocation());
    else
      // Remember that we parsed a Block type, and remember the type-quals.
      D.AddTypeInfo(DeclaratorChunk::getBlockPointer(DS.getTypeQualifiers(),
                                                     Loc),
                    DS.getAttributes(),
                    SourceLocation());
  } else {
    // Is a reference
    DeclSpec DS(AttrFactory);

    // Complain about rvalue references in C++03, but then go on and build
    // the declarator.
    if (Kind == tok::ampamp)
      Diag(Loc, getLangOpts().CPlusPlus11 ?
           diag::warn_cxx98_compat_rvalue_reference :
           diag::ext_rvalue_reference);

    // GNU-style and C++11 attributes are allowed here, as is restrict.
    ParseTypeQualifierListOpt(DS);
    D.ExtendWithDeclSpec(DS);

    // C++ 8.3.2p1: cv-qualified references are ill-formed except when the
    // cv-qualifiers are introduced through the use of a typedef or of a
    // template type argument, in which case the cv-qualifiers are ignored.
    if (DS.getTypeQualifiers() != DeclSpec::TQ_unspecified) {
      if (DS.getTypeQualifiers() & DeclSpec::TQ_const)
        Diag(DS.getConstSpecLoc(),
             diag::err_invalid_reference_qualifier_application) << "const";
      if (DS.getTypeQualifiers() & DeclSpec::TQ_volatile)
        Diag(DS.getVolatileSpecLoc(),
             diag::err_invalid_reference_qualifier_application) << "volatile";
      // 'restrict' is permitted as an extension.
      if (DS.getTypeQualifiers() & DeclSpec::TQ_atomic)
        Diag(DS.getAtomicSpecLoc(),
             diag::err_invalid_reference_qualifier_application) << "_Atomic";
    }

    // Recursively parse the declarator.
    ParseDeclaratorInternal(D, DirectDeclParser);

    if (D.getNumTypeObjects() > 0) {
      // C++ [dcl.ref]p4: There shall be no references to references.
      DeclaratorChunk& InnerChunk = D.getTypeObject(D.getNumTypeObjects() - 1);
      if (InnerChunk.Kind == DeclaratorChunk::Reference) {
        if (const IdentifierInfo *II = D.getIdentifier())
          Diag(InnerChunk.Loc, diag::err_illegal_decl_reference_to_reference)
           << II;
        else
          Diag(InnerChunk.Loc, diag::err_illegal_decl_reference_to_reference)
            << "type name";

        // Once we've complained about the reference-to-reference, we
        // can go ahead and build the (technically ill-formed)
        // declarator: reference collapsing will take care of it.
      }
    }

    // Remember that we parsed a reference type.
    D.AddTypeInfo(DeclaratorChunk::getReference(DS.getTypeQualifiers(), Loc,
                                                Kind == tok::amp),
                  DS.getAttributes(),
                  SourceLocation());
  }
}

static void diagnoseMisplacedEllipsis(Parser &P, Declarator &D,
                                      SourceLocation EllipsisLoc) {
  if (EllipsisLoc.isValid()) {
    FixItHint Insertion;
    if (!D.getEllipsisLoc().isValid()) {
      Insertion = FixItHint::CreateInsertion(D.getIdentifierLoc(), "...");
      D.setEllipsisLoc(EllipsisLoc);
    }
    P.Diag(EllipsisLoc, diag::err_misplaced_ellipsis_in_declaration)
      << FixItHint::CreateRemoval(EllipsisLoc) << Insertion << !D.hasName();
  }
}

/// ParseDirectDeclarator
///       direct-declarator: [C99 6.7.5]
/// [C99]   identifier
///         '(' declarator ')'
/// [GNU]   '(' attributes declarator ')'
/// [C90]   direct-declarator '[' constant-expression[opt] ']'
/// [C99]   direct-declarator '[' type-qual-list[opt] assignment-expr[opt] ']'
/// [C99]   direct-declarator '[' 'static' type-qual-list[opt] assign-expr ']'
/// [C99]   direct-declarator '[' type-qual-list 'static' assignment-expr ']'
/// [C99]   direct-declarator '[' type-qual-list[opt] '*' ']'
/// [C++11] direct-declarator '[' constant-expression[opt] ']'
///                    attribute-specifier-seq[opt]
///         direct-declarator '(' parameter-type-list ')'
///         direct-declarator '(' identifier-list[opt] ')'
/// [GNU]   direct-declarator '(' parameter-forward-declarations
///                    parameter-type-list[opt] ')'
/// [C++]   direct-declarator '(' parameter-declaration-clause ')'
///                    cv-qualifier-seq[opt] exception-specification[opt]
/// [C++11] direct-declarator '(' parameter-declaration-clause ')'
///                    attribute-specifier-seq[opt] cv-qualifier-seq[opt]
///                    ref-qualifier[opt] exception-specification[opt]
/// [C++]   declarator-id
/// [C++11] declarator-id attribute-specifier-seq[opt]
///
///       declarator-id: [C++ 8]
///         '...'[opt] id-expression
///         '::'[opt] nested-name-specifier[opt] type-name
///
///       id-expression: [C++ 5.1]
///         unqualified-id
///         qualified-id
///
///       unqualified-id: [C++ 5.1]
///         identifier
///         operator-function-id
///         conversion-function-id
///          '~' class-name
///         template-id
///
/// Note, any additional constructs added here may need corresponding changes
/// in isConstructorDeclarator.
void Parser::ParseDirectDeclarator(Declarator &D) {
  DeclaratorScopeObj DeclScopeObj(*this, D.getCXXScopeSpec());

  if (getLangOpts().CPlusPlus && D.mayHaveIdentifier()) {
    // ParseDeclaratorInternal might already have parsed the scope.
    if (D.getCXXScopeSpec().isEmpty()) {
      bool EnteringContext = D.getContext() == Declarator::FileContext ||
                             D.getContext() == Declarator::MemberContext;
      ParseOptionalCXXScopeSpecifier(D.getCXXScopeSpec(), ParsedType(),
                                     EnteringContext);
    }

    if (D.getCXXScopeSpec().isValid()) {
      if (Actions.ShouldEnterDeclaratorScope(getCurScope(), D.getCXXScopeSpec()))
        // Change the declaration context for name lookup, until this function
        // is exited (and the declarator has been parsed).
        DeclScopeObj.EnterDeclaratorScope();
    }

    // C++0x [dcl.fct]p14:
    //   There is a syntactic ambiguity when an ellipsis occurs at the end
    //   of a parameter-declaration-clause without a preceding comma. In
    //   this case, the ellipsis is parsed as part of the
    //   abstract-declarator if the type of the parameter names a template
    //   parameter pack that has not been expanded; otherwise, it is parsed
    //   as part of the parameter-declaration-clause.
    if (Tok.is(tok::ellipsis) && D.getCXXScopeSpec().isEmpty() &&
        !((D.getContext() == Declarator::PrototypeContext ||
           D.getContext() == Declarator::LambdaExprParameterContext ||
           D.getContext() == Declarator::BlockLiteralContext) &&
          NextToken().is(tok::r_paren) &&
          !D.hasGroupingParens() &&
          !Actions.containsUnexpandedParameterPacks(D))) {
      SourceLocation EllipsisLoc = ConsumeToken();
      if (isPtrOperatorToken(Tok.getKind(), getLangOpts())) {
        // The ellipsis was put in the wrong place. Recover, and explain to
        // the user what they should have done.
        ParseDeclarator(D);
        diagnoseMisplacedEllipsis(*this, D, EllipsisLoc);
        return;
      } else
        D.setEllipsisLoc(EllipsisLoc);

      // The ellipsis can't be followed by a parenthesized declarator. We
      // check for that in ParseParenDeclarator, after we have disambiguated
      // the l_paren token.
    }

    if (Tok.is(tok::identifier) || Tok.is(tok::kw_operator) ||
        Tok.is(tok::annot_template_id) || Tok.is(tok::tilde)) {
      // We found something that indicates the start of an unqualified-id.
      // Parse that unqualified-id.
      bool AllowConstructorName;
      if (D.getDeclSpec().hasTypeSpecifier())
        AllowConstructorName = false;
      else if (D.getCXXScopeSpec().isSet())
        AllowConstructorName =
          (D.getContext() == Declarator::FileContext ||
           D.getContext() == Declarator::MemberContext);
      else
        AllowConstructorName = (D.getContext() == Declarator::MemberContext);

      SourceLocation TemplateKWLoc;
      if (ParseUnqualifiedId(D.getCXXScopeSpec(),
                             /*EnteringContext=*/true,
                             /*AllowDestructorName=*/true,
                             AllowConstructorName,
                             ParsedType(),
                             TemplateKWLoc,
                             D.getName()) ||
          // Once we're past the identifier, if the scope was bad, mark the
          // whole declarator bad.
          D.getCXXScopeSpec().isInvalid()) {
        D.SetIdentifier(0, Tok.getLocation());
        D.setInvalidType(true);
      } else {
        // Parsed the unqualified-id; update range information and move along.
        if (D.getSourceRange().getBegin().isInvalid())
          D.SetRangeBegin(D.getName().getSourceRange().getBegin());
        D.SetRangeEnd(D.getName().getSourceRange().getEnd());
      }
      goto PastIdentifier;
    }
  } else if (Tok.is(tok::identifier) && D.mayHaveIdentifier()) {
    assert(!getLangOpts().CPlusPlus &&
           "There's a C++-specific check for tok::identifier above");
    assert(Tok.getIdentifierInfo() && "Not an identifier?");
    D.SetIdentifier(Tok.getIdentifierInfo(), Tok.getLocation());
    ConsumeToken();
    goto PastIdentifier;
  } else if (Tok.is(tok::identifier) && D.diagnoseIdentifier()) {
    // A virt-specifier isn't treated as an identifier if it appears after a
    // trailing-return-type.
    if (D.getContext() != Declarator::TrailingReturnContext ||
        !isCXX11VirtSpecifier(Tok)) {
      Diag(Tok.getLocation(), diag::err_unexpected_unqualified_id)
        << FixItHint::CreateRemoval(Tok.getLocation());
      D.SetIdentifier(0, Tok.getLocation());
      ConsumeToken();
      goto PastIdentifier;
    }
  }

  if (Tok.is(tok::l_paren)) {
    // direct-declarator: '(' declarator ')'
    // direct-declarator: '(' attributes declarator ')'
    // Example: 'char (*X)'   or 'int (*XX)(void)'
    ParseParenDeclarator(D);

    // If the declarator was parenthesized, we entered the declarator
    // scope when parsing the parenthesized declarator, then exited
    // the scope already. Re-enter the scope, if we need to.
    if (D.getCXXScopeSpec().isSet()) {
      // If there was an error parsing parenthesized declarator, declarator
      // scope may have been entered before. Don't do it again.
      if (!D.isInvalidType() &&
          Actions.ShouldEnterDeclaratorScope(getCurScope(), D.getCXXScopeSpec()))
        // Change the declaration context for name lookup, until this function
        // is exited (and the declarator has been parsed).
        DeclScopeObj.EnterDeclaratorScope();
    }
  } else if (D.mayOmitIdentifier()) {
    // This could be something simple like "int" (in which case the declarator
    // portion is empty), if an abstract-declarator is allowed.
    D.SetIdentifier(0, Tok.getLocation());

    // The grammar for abstract-pack-declarator does not allow grouping parens.
    // FIXME: Revisit this once core issue 1488 is resolved.
    if (D.hasEllipsis() && D.hasGroupingParens())
      Diag(PP.getLocForEndOfToken(D.getEllipsisLoc()),
           diag::ext_abstract_pack_declarator_parens);
  } else {
    if (Tok.getKind() == tok::annot_pragma_parser_crash)
      LLVM_BUILTIN_TRAP;
    if (D.getContext() == Declarator::MemberContext)
      Diag(Tok, diag::err_expected_member_name_or_semi)
        << D.getDeclSpec().getSourceRange();
    else if (getLangOpts().CPlusPlus) {
      if (Tok.is(tok::period) || Tok.is(tok::arrow))
        Diag(Tok, diag::err_invalid_operator_on_type) << Tok.is(tok::arrow);
      else {
        SourceLocation Loc = D.getCXXScopeSpec().getEndLoc();
        if (Tok.isAtStartOfLine() && Loc.isValid())
          Diag(PP.getLocForEndOfToken(Loc), diag::err_expected_unqualified_id)
              << getLangOpts().CPlusPlus;
        else
          Diag(Tok, diag::err_expected_unqualified_id)
              << getLangOpts().CPlusPlus;
      }
    } else
      Diag(Tok, diag::err_expected_either) << tok::identifier << tok::l_paren;
    D.SetIdentifier(0, Tok.getLocation());
    D.setInvalidType(true);
  }

 PastIdentifier:
  assert(D.isPastIdentifier() &&
         "Haven't past the location of the identifier yet?");

  // Don't parse attributes unless we have parsed an unparenthesized name.
  if (D.hasName() && !D.getNumTypeObjects())
    MaybeParseCXX11Attributes(D);

  while (1) {
    if (Tok.is(tok::l_paren)) {
      // Enter function-declaration scope, limiting any declarators to the
      // function prototype scope, including parameter declarators.
      ParseScope PrototypeScope(this,
                                Scope::FunctionPrototypeScope|Scope::DeclScope|
                                (D.isFunctionDeclaratorAFunctionDeclaration()
                                   ? Scope::FunctionDeclarationScope : 0));

      // The paren may be part of a C++ direct initializer, eg. "int x(1);".
      // In such a case, check if we actually have a function declarator; if it
      // is not, the declarator has been fully parsed.
      bool IsAmbiguous = false;
      if (getLangOpts().CPlusPlus && D.mayBeFollowedByCXXDirectInit()) {
        // The name of the declarator, if any, is tentatively declared within
        // a possible direct initializer.
        TentativelyDeclaredIdentifiers.push_back(D.getIdentifier());
        bool IsFunctionDecl = isCXXFunctionDeclarator(&IsAmbiguous);
        TentativelyDeclaredIdentifiers.pop_back();
        if (!IsFunctionDecl)
          break;
      }
      ParsedAttributes attrs(AttrFactory);
      BalancedDelimiterTracker T(*this, tok::l_paren);
      T.consumeOpen();
      ParseFunctionDeclarator(D, attrs, T, IsAmbiguous);
      PrototypeScope.Exit();
    } else if (Tok.is(tok::l_square)) {
      ParseBracketDeclarator(D);
    } else {
      break;
    }
  }
}

/// ParseParenDeclarator - We parsed the declarator D up to a paren.  This is
/// only called before the identifier, so these are most likely just grouping
/// parens for precedence.  If we find that these are actually function
/// parameter parens in an abstract-declarator, we call ParseFunctionDeclarator.
///
///       direct-declarator:
///         '(' declarator ')'
/// [GNU]   '(' attributes declarator ')'
///         direct-declarator '(' parameter-type-list ')'
///         direct-declarator '(' identifier-list[opt] ')'
/// [GNU]   direct-declarator '(' parameter-forward-declarations
///                    parameter-type-list[opt] ')'
///
void Parser::ParseParenDeclarator(Declarator &D) {
  BalancedDelimiterTracker T(*this, tok::l_paren);
  T.consumeOpen();

  assert(!D.isPastIdentifier() && "Should be called before passing identifier");

  // Eat any attributes before we look at whether this is a grouping or function
  // declarator paren.  If this is a grouping paren, the attribute applies to
  // the type being built up, for example:
  //     int (__attribute__(()) *x)(long y)
  // If this ends up not being a grouping paren, the attribute applies to the
  // first argument, for example:
  //     int (__attribute__(()) int x)
  // In either case, we need to eat any attributes to be able to determine what
  // sort of paren this is.
  //
  ParsedAttributes attrs(AttrFactory);
  bool RequiresArg = false;
  if (Tok.is(tok::kw___attribute)) {
    ParseGNUAttributes(attrs);

    // We require that the argument list (if this is a non-grouping paren) be
    // present even if the attribute list was empty.
    RequiresArg = true;
  }

  // Eat any Microsoft extensions.
  ParseMicrosoftTypeAttributes(attrs);

  // Eat any Borland extensions.
  if  (Tok.is(tok::kw___pascal))
    ParseBorlandTypeAttributes(attrs);

  // If we haven't past the identifier yet (or where the identifier would be
  // stored, if this is an abstract declarator), then this is probably just
  // grouping parens. However, if this could be an abstract-declarator, then
  // this could also be the start of function arguments (consider 'void()').
  bool isGrouping;

  if (!D.mayOmitIdentifier()) {
    // If this can't be an abstract-declarator, this *must* be a grouping
    // paren, because we haven't seen the identifier yet.
    isGrouping = true;
  } else if (Tok.is(tok::r_paren) ||           // 'int()' is a function.
             (getLangOpts().CPlusPlus && Tok.is(tok::ellipsis) &&
              NextToken().is(tok::r_paren)) || // C++ int(...)
             isDeclarationSpecifier() ||       // 'int(int)' is a function.
             isCXX11AttributeSpecifier()) {    // 'int([[]]int)' is a function.
    // This handles C99 6.7.5.3p11: in "typedef int X; void foo(X)", X is
    // considered to be a type, not a K&R identifier-list.
    isGrouping = false;
  } else {
    // Otherwise, this is a grouping paren, e.g. 'int (*X)' or 'int(X)'.
    isGrouping = true;
  }

  // If this is a grouping paren, handle:
  // direct-declarator: '(' declarator ')'
  // direct-declarator: '(' attributes declarator ')'
  if (isGrouping) {
    SourceLocation EllipsisLoc = D.getEllipsisLoc();
    D.setEllipsisLoc(SourceLocation());

    bool hadGroupingParens = D.hasGroupingParens();
    D.setGroupingParens(true);
    ParseDeclaratorInternal(D, &Parser::ParseDirectDeclarator);
    // Match the ')'.
    T.consumeClose();
    D.AddTypeInfo(DeclaratorChunk::getParen(T.getOpenLocation(),
                                            T.getCloseLocation()),
                  attrs, T.getCloseLocation());

    D.setGroupingParens(hadGroupingParens);

    // An ellipsis cannot be placed outside parentheses.
    if (EllipsisLoc.isValid())
      diagnoseMisplacedEllipsis(*this, D, EllipsisLoc);

    return;
  }

  // Okay, if this wasn't a grouping paren, it must be the start of a function
  // argument list.  Recognize that this declarator will never have an
  // identifier (and remember where it would have been), then call into
  // ParseFunctionDeclarator to handle of argument list.
  D.SetIdentifier(0, Tok.getLocation());

  // Enter function-declaration scope, limiting any declarators to the
  // function prototype scope, including parameter declarators.
  ParseScope PrototypeScope(this,
                            Scope::FunctionPrototypeScope | Scope::DeclScope |
                            (D.isFunctionDeclaratorAFunctionDeclaration()
                               ? Scope::FunctionDeclarationScope : 0));
  ParseFunctionDeclarator(D, attrs, T, false, RequiresArg);
  PrototypeScope.Exit();
}

/// ParseFunctionDeclarator - We are after the identifier and have parsed the
/// declarator D up to a paren, which indicates that we are parsing function
/// arguments.
///
/// If FirstArgAttrs is non-null, then the caller parsed those arguments
/// immediately after the open paren - they should be considered to be the
/// first argument of a parameter.
///
/// If RequiresArg is true, then the first argument of the function is required
/// to be present and required to not be an identifier list.
///
/// For C++, after the parameter-list, it also parses the cv-qualifier-seq[opt],
/// (C++11) ref-qualifier[opt], exception-specification[opt],
/// (C++11) attribute-specifier-seq[opt], and (C++11) trailing-return-type[opt].
///
/// [C++11] exception-specification:
///           dynamic-exception-specification
///           noexcept-specification
///
void Parser::ParseFunctionDeclarator(Declarator &D,
                                     ParsedAttributes &FirstArgAttrs,
                                     BalancedDelimiterTracker &Tracker,
                                     bool IsAmbiguous,
                                     bool RequiresArg) {
  assert(getCurScope()->isFunctionPrototypeScope() &&
         "Should call from a Function scope");
  // lparen is already consumed!
  assert(D.isPastIdentifier() && "Should not call before identifier!");

  // This should be true when the function has typed arguments.
  // Otherwise, it is treated as a K&R-style function.
  bool HasProto = false;
  // Build up an array of information about the parsed arguments.
  SmallVector<DeclaratorChunk::ParamInfo, 16> ParamInfo;
  // Remember where we see an ellipsis, if any.
  SourceLocation EllipsisLoc;

  DeclSpec DS(AttrFactory);
  bool RefQualifierIsLValueRef = true;
  SourceLocation RefQualifierLoc;
  SourceLocation ConstQualifierLoc;
  SourceLocation VolatileQualifierLoc;
  ExceptionSpecificationType ESpecType = EST_None;
  SourceRange ESpecRange;
  SmallVector<ParsedType, 2> DynamicExceptions;
  SmallVector<SourceRange, 2> DynamicExceptionRanges;
  ExprResult NoexceptExpr;
  ParsedAttributes FnAttrs(AttrFactory);
  TypeResult TrailingReturnType;

  /* LocalEndLoc is the end location for the local FunctionTypeLoc.
     EndLoc is the end location for the function declarator.
     They differ for trailing return types. */
  SourceLocation StartLoc, LocalEndLoc, EndLoc;
  SourceLocation LParenLoc, RParenLoc;
  LParenLoc = Tracker.getOpenLocation();
  StartLoc = LParenLoc;

  if (isFunctionDeclaratorIdentifierList()) {
    if (RequiresArg)
      Diag(Tok, diag::err_argument_required_after_attribute);

    ParseFunctionDeclaratorIdentifierList(D, ParamInfo);

    Tracker.consumeClose();
    RParenLoc = Tracker.getCloseLocation();
    LocalEndLoc = RParenLoc;
    EndLoc = RParenLoc;
  } else {
    if (Tok.isNot(tok::r_paren))
      ParseParameterDeclarationClause(D, FirstArgAttrs, ParamInfo, 
                                      EllipsisLoc);
    else if (RequiresArg)
      Diag(Tok, diag::err_argument_required_after_attribute);

    HasProto = ParamInfo.size() || getLangOpts().CPlusPlus;

    // If we have the closing ')', eat it.
    Tracker.consumeClose();
    RParenLoc = Tracker.getCloseLocation();
    LocalEndLoc = RParenLoc;
    EndLoc = RParenLoc;

    if (getLangOpts().CPlusPlus) {
      // FIXME: Accept these components in any order, and produce fixits to
      // correct the order if the user gets it wrong. Ideally we should deal
      // with the virt-specifier-seq and pure-specifier in the same way.

      // Parse cv-qualifier-seq[opt].
      ParseTypeQualifierListOpt(DS, /*VendorAttributesAllowed*/ false,
                                /*CXX11AttributesAllowed*/ false,
                                /*AtomicAllowed*/ false);
      if (!DS.getSourceRange().getEnd().isInvalid()) {
        EndLoc = DS.getSourceRange().getEnd();
        ConstQualifierLoc = DS.getConstSpecLoc();
        VolatileQualifierLoc = DS.getVolatileSpecLoc();
      }

      // Parse ref-qualifier[opt].
      if (Tok.is(tok::amp) || Tok.is(tok::ampamp)) {
        Diag(Tok, getLangOpts().CPlusPlus11 ?
             diag::warn_cxx98_compat_ref_qualifier :
             diag::ext_ref_qualifier);

        RefQualifierIsLValueRef = Tok.is(tok::amp);
        RefQualifierLoc = ConsumeToken();
        EndLoc = RefQualifierLoc;
      }

      // C++11 [expr.prim.general]p3:
      //   If a declaration declares a member function or member function
      //   template of a class X, the expression this is a prvalue of type
      //   "pointer to cv-qualifier-seq X" between the optional cv-qualifer-seq
      //   and the end of the function-definition, member-declarator, or
      //   declarator.
      // FIXME: currently, "static" case isn't handled correctly.
      bool IsCXX11MemberFunction =
        getLangOpts().CPlusPlus11 &&
        D.getDeclSpec().getStorageClassSpec() != DeclSpec::SCS_typedef &&
        (D.getContext() == Declarator::MemberContext
         ? !D.getDeclSpec().isFriendSpecified()
         : D.getContext() == Declarator::FileContext &&
           D.getCXXScopeSpec().isValid() &&
           Actions.CurContext->isRecord());
      Sema::CXXThisScopeRAII ThisScope(Actions,
                               dyn_cast<CXXRecordDecl>(Actions.CurContext),
                               DS.getTypeQualifiers() |
                               (D.getDeclSpec().isConstexprSpecified() &&
                                !getLangOpts().CPlusPlus1y
                                  ? Qualifiers::Const : 0),
                               IsCXX11MemberFunction);

      // Parse exception-specification[opt].
      ESpecType = tryParseExceptionSpecification(ESpecRange,
                                                 DynamicExceptions,
                                                 DynamicExceptionRanges,
                                                 NoexceptExpr);
      if (ESpecType != EST_None)
        EndLoc = ESpecRange.getEnd();

      // Parse attribute-specifier-seq[opt]. Per DR 979 and DR 1297, this goes
      // after the exception-specification.
      MaybeParseCXX11Attributes(FnAttrs);

      // Parse trailing-return-type[opt].
      LocalEndLoc = EndLoc;
      if (getLangOpts().CPlusPlus11 && Tok.is(tok::arrow)) {
        Diag(Tok, diag::warn_cxx98_compat_trailing_return_type);
        if (D.getDeclSpec().getTypeSpecType() == TST_auto)
          StartLoc = D.getDeclSpec().getTypeSpecTypeLoc();
        LocalEndLoc = Tok.getLocation();
        SourceRange Range;
        TrailingReturnType = ParseTrailingReturnType(Range);
        EndLoc = Range.getEnd();
      }
    }
  }

  // Remember that we parsed a function type, and remember the attributes.
  D.AddTypeInfo(DeclaratorChunk::getFunction(HasProto,
                                             IsAmbiguous,
                                             LParenLoc,
                                             ParamInfo.data(), ParamInfo.size(),
                                             EllipsisLoc, RParenLoc,
                                             DS.getTypeQualifiers(),
                                             RefQualifierIsLValueRef,
                                             RefQualifierLoc, ConstQualifierLoc,
                                             VolatileQualifierLoc,
                                             /*MutableLoc=*/SourceLocation(),
                                             ESpecType, ESpecRange.getBegin(),
                                             DynamicExceptions.data(),
                                             DynamicExceptionRanges.data(),
                                             DynamicExceptions.size(),
                                             NoexceptExpr.isUsable() ?
                                               NoexceptExpr.get() : 0,
                                             StartLoc, LocalEndLoc, D,
                                             TrailingReturnType),
                FnAttrs, EndLoc);
}

/// isFunctionDeclaratorIdentifierList - This parameter list may have an
/// identifier list form for a K&R-style function:  void foo(a,b,c)
///
/// Note that identifier-lists are only allowed for normal declarators, not for
/// abstract-declarators.
bool Parser::isFunctionDeclaratorIdentifierList() {
  return !getLangOpts().CPlusPlus
         && Tok.is(tok::identifier)
         && !TryAltiVecVectorToken()
         // K&R identifier lists can't have typedefs as identifiers, per C99
         // 6.7.5.3p11.
         && (TryAnnotateTypeOrScopeToken() || !Tok.is(tok::annot_typename))
         // Identifier lists follow a really simple grammar: the identifiers can
         // be followed *only* by a ", identifier" or ")".  However, K&R
         // identifier lists are really rare in the brave new modern world, and
         // it is very common for someone to typo a type in a non-K&R style
         // list.  If we are presented with something like: "void foo(intptr x,
         // float y)", we don't want to start parsing the function declarator as
         // though it is a K&R style declarator just because intptr is an
         // invalid type.
         //
         // To handle this, we check to see if the token after the first
         // identifier is a "," or ")".  Only then do we parse it as an
         // identifier list.
         && (NextToken().is(tok::comma) || NextToken().is(tok::r_paren));
}

/// ParseFunctionDeclaratorIdentifierList - While parsing a function declarator
/// we found a K&R-style identifier list instead of a typed parameter list.
///
/// After returning, ParamInfo will hold the parsed parameters.
///
///       identifier-list: [C99 6.7.5]
///         identifier
///         identifier-list ',' identifier
///
void Parser::ParseFunctionDeclaratorIdentifierList(
       Declarator &D,
       SmallVectorImpl<DeclaratorChunk::ParamInfo> &ParamInfo) {
  // If there was no identifier specified for the declarator, either we are in
  // an abstract-declarator, or we are in a parameter declarator which was found
  // to be abstract.  In abstract-declarators, identifier lists are not valid:
  // diagnose this.
  if (!D.getIdentifier())
    Diag(Tok, diag::ext_ident_list_in_param);

  // Maintain an efficient lookup of params we have seen so far.
  llvm::SmallSet<const IdentifierInfo*, 16> ParamsSoFar;

  do {
    // If this isn't an identifier, report the error and skip until ')'.
    if (Tok.isNot(tok::identifier)) {
      Diag(Tok, diag::err_expected) << tok::identifier;
      SkipUntil(tok::r_paren, StopAtSemi | StopBeforeMatch);
      // Forget we parsed anything.
      ParamInfo.clear();
      return;
    }

    IdentifierInfo *ParmII = Tok.getIdentifierInfo();

    // Reject 'typedef int y; int test(x, y)', but continue parsing.
    if (Actions.getTypeName(*ParmII, Tok.getLocation(), getCurScope()))
      Diag(Tok, diag::err_unexpected_typedef_ident) << ParmII;

    // Verify that the argument identifier has not already been mentioned.
    if (!ParamsSoFar.insert(ParmII)) {
      Diag(Tok, diag::err_param_redefinition) << ParmII;
    } else {
      // Remember this identifier in ParamInfo.
      ParamInfo.push_back(DeclaratorChunk::ParamInfo(ParmII,
                                                     Tok.getLocation(),
                                                     0));
    }

    // Eat the identifier.
    ConsumeToken();
    // The list continues if we see a comma.
  } while (TryConsumeToken(tok::comma));
}

/// ParseParameterDeclarationClause - Parse a (possibly empty) parameter-list
/// after the opening parenthesis. This function will not parse a K&R-style
/// identifier list.
///
/// D is the declarator being parsed.  If FirstArgAttrs is non-null, then the
/// caller parsed those arguments immediately after the open paren - they should
/// be considered to be part of the first parameter.
///
/// After returning, ParamInfo will hold the parsed parameters. EllipsisLoc will
/// be the location of the ellipsis, if any was parsed.
///
///       parameter-type-list: [C99 6.7.5]
///         parameter-list
///         parameter-list ',' '...'
/// [C++]   parameter-list '...'
///
///       parameter-list: [C99 6.7.5]
///         parameter-declaration
///         parameter-list ',' parameter-declaration
///
///       parameter-declaration: [C99 6.7.5]
///         declaration-specifiers declarator
/// [C++]   declaration-specifiers declarator '=' assignment-expression
/// [C++11]                                       initializer-clause
/// [GNU]   declaration-specifiers declarator attributes
///         declaration-specifiers abstract-declarator[opt]
/// [C++]   declaration-specifiers abstract-declarator[opt]
///           '=' assignment-expression
/// [GNU]   declaration-specifiers abstract-declarator[opt] attributes
/// [C++11] attribute-specifier-seq parameter-declaration
///
void Parser::ParseParameterDeclarationClause(
       Declarator &D,
       ParsedAttributes &FirstArgAttrs,
       SmallVectorImpl<DeclaratorChunk::ParamInfo> &ParamInfo,
       SourceLocation &EllipsisLoc) {
  do {
    // FIXME: Issue a diagnostic if we parsed an attribute-specifier-seq
    // before deciding this was a parameter-declaration-clause.
    if (TryConsumeToken(tok::ellipsis, EllipsisLoc))
      break;

    // Parse the declaration-specifiers.
    // Just use the ParsingDeclaration "scope" of the declarator.
    DeclSpec DS(AttrFactory);

    // Parse any C++11 attributes.
    MaybeParseCXX11Attributes(DS.getAttributes());

    // Skip any Microsoft attributes before a param.
    MaybeParseMicrosoftAttributes(DS.getAttributes());

    SourceLocation DSStart = Tok.getLocation();

    // If the caller parsed attributes for the first argument, add them now.
    // Take them so that we only apply the attributes to the first parameter.
    // FIXME: If we can leave the attributes in the token stream somehow, we can
    // get rid of a parameter (FirstArgAttrs) and this statement. It might be
    // too much hassle.
    DS.takeAttributesFrom(FirstArgAttrs);

    ParseDeclarationSpecifiers(DS);


    // Parse the declarator.  This is "PrototypeContext" or 
    // "LambdaExprParameterContext", because we must accept either 
    // 'declarator' or 'abstract-declarator' here.
    Declarator ParmDeclarator(DS, 
              D.getContext() == Declarator::LambdaExprContext ?
                                  Declarator::LambdaExprParameterContext : 
                                                Declarator::PrototypeContext);
    ParseDeclarator(ParmDeclarator);

    // Parse GNU attributes, if present.
    MaybeParseGNUAttributes(ParmDeclarator);

    // Remember this parsed parameter in ParamInfo.
    IdentifierInfo *ParmII = ParmDeclarator.getIdentifier();

    // DefArgToks is used when the parsing of default arguments needs
    // to be delayed.
    CachedTokens *DefArgToks = 0;

    // If no parameter was specified, verify that *something* was specified,
    // otherwise we have a missing type and identifier.
    if (DS.isEmpty() && ParmDeclarator.getIdentifier() == 0 &&
        ParmDeclarator.getNumTypeObjects() == 0) {
      // Completely missing, emit error.
      Diag(DSStart, diag::err_missing_param);
    } else {
      // Otherwise, we have something.  Add it and let semantic analysis try
      // to grok it and add the result to the ParamInfo we are building.

      // Inform the actions module about the parameter declarator, so it gets
      // added to the current scope.
      Decl *Param = Actions.ActOnParamDeclarator(getCurScope(), 
                                                       ParmDeclarator);
      // Parse the default argument, if any. We parse the default
      // arguments in all dialects; the semantic analysis in
      // ActOnParamDefaultArgument will reject the default argument in
      // C.
      if (Tok.is(tok::equal)) {
        SourceLocation EqualLoc = Tok.getLocation();

        // Parse the default argument
        if (D.getContext() == Declarator::MemberContext) {
          // If we're inside a class definition, cache the tokens
          // corresponding to the default argument. We'll actually parse
          // them when we see the end of the class definition.
          // FIXME: Can we use a smart pointer for Toks?
          DefArgToks = new CachedTokens;

          if (!ConsumeAndStoreInitializer(*DefArgToks, CIK_DefaultArgument)) {
            delete DefArgToks;
            DefArgToks = 0;
            Actions.ActOnParamDefaultArgumentError(Param);
          } else {
            // Mark the end of the default argument so that we know when to
            // stop when we parse it later on.
            Token DefArgEnd;
            DefArgEnd.startToken();
            DefArgEnd.setKind(tok::cxx_defaultarg_end);
            DefArgEnd.setLocation(Tok.getLocation());
            DefArgToks->push_back(DefArgEnd);
            Actions.ActOnParamUnparsedDefaultArgument(Param, EqualLoc,
                                                (*DefArgToks)[1].getLocation());
          }
        } else {
          // Consume the '='.
          ConsumeToken();

          // The argument isn't actually potentially evaluated unless it is
          // used.
          EnterExpressionEvaluationContext Eval(Actions,
                                              Sema::PotentiallyEvaluatedIfUsed,
                                                Param);

          ExprResult DefArgResult;
          if (getLangOpts().CPlusPlus11 && Tok.is(tok::l_brace)) {
            Diag(Tok, diag::warn_cxx98_compat_generalized_initializer_lists);
            DefArgResult = ParseBraceInitializer();
          } else
            DefArgResult = ParseAssignmentExpression();
          if (DefArgResult.isInvalid()) {
            Actions.ActOnParamDefaultArgumentError(Param);
            SkipUntil(tok::comma, tok::r_paren, StopAtSemi | StopBeforeMatch);
          } else {
            // Inform the actions module about the default argument
            Actions.ActOnParamDefaultArgument(Param, EqualLoc,
                                              DefArgResult.take());
          }
        }
      }

      ParamInfo.push_back(DeclaratorChunk::ParamInfo(ParmII,
                                          ParmDeclarator.getIdentifierLoc(), 
                                          Param, DefArgToks));
    }

    if (TryConsumeToken(tok::ellipsis, EllipsisLoc) &&
        !getLangOpts().CPlusPlus) {
      // We have ellipsis without a preceding ',', which is ill-formed
      // in C. Complain and provide the fix.
      Diag(EllipsisLoc, diag::err_missing_comma_before_ellipsis)
          << FixItHint::CreateInsertion(EllipsisLoc, ", ");
      break;
    }

    // If the next token is a comma, consume it and keep reading arguments.
  } while (TryConsumeToken(tok::comma));
}

/// [C90]   direct-declarator '[' constant-expression[opt] ']'
/// [C99]   direct-declarator '[' type-qual-list[opt] assignment-expr[opt] ']'
/// [C99]   direct-declarator '[' 'static' type-qual-list[opt] assign-expr ']'
/// [C99]   direct-declarator '[' type-qual-list 'static' assignment-expr ']'
/// [C99]   direct-declarator '[' type-qual-list[opt] '*' ']'
/// [C++11] direct-declarator '[' constant-expression[opt] ']'
///                           attribute-specifier-seq[opt]
void Parser::ParseBracketDeclarator(Declarator &D) {
  if (CheckProhibitedCXX11Attribute())
    return;

  BalancedDelimiterTracker T(*this, tok::l_square);
  T.consumeOpen();

  // C array syntax has many features, but by-far the most common is [] and [4].
  // This code does a fast path to handle some of the most obvious cases.
  if (Tok.getKind() == tok::r_square) {
    T.consumeClose();
    ParsedAttributes attrs(AttrFactory);
    MaybeParseCXX11Attributes(attrs);

    // Remember that we parsed the empty array type.
    D.AddTypeInfo(DeclaratorChunk::getArray(0, false, false, 0,
                                            T.getOpenLocation(),
                                            T.getCloseLocation()),
                  attrs, T.getCloseLocation());
    return;
  } else if (Tok.getKind() == tok::numeric_constant &&
             GetLookAheadToken(1).is(tok::r_square)) {
    // [4] is very common.  Parse the numeric constant expression.
    ExprResult ExprRes(Actions.ActOnNumericConstant(Tok, getCurScope()));
    ConsumeToken();

    T.consumeClose();
    ParsedAttributes attrs(AttrFactory);
    MaybeParseCXX11Attributes(attrs);

    // Remember that we parsed a array type, and remember its features.
    D.AddTypeInfo(DeclaratorChunk::getArray(0, false, false,
                                            ExprRes.release(),
                                            T.getOpenLocation(),
                                            T.getCloseLocation()),
                  attrs, T.getCloseLocation());
    return;
  }

  // If valid, this location is the position where we read the 'static' keyword.
  SourceLocation StaticLoc;
  TryConsumeToken(tok::kw_static, StaticLoc);

  // If there is a type-qualifier-list, read it now.
  // Type qualifiers in an array subscript are a C99 feature.
  DeclSpec DS(AttrFactory);
  ParseTypeQualifierListOpt(DS, false /*no attributes*/);

  // If we haven't already read 'static', check to see if there is one after the
  // type-qualifier-list.
  if (!StaticLoc.isValid())
    TryConsumeToken(tok::kw_static, StaticLoc);

  // Handle "direct-declarator [ type-qual-list[opt] * ]".
  bool isStar = false;
  ExprResult NumElements;

  // Handle the case where we have '[*]' as the array size.  However, a leading
  // star could be the start of an expression, for example 'X[*p + 4]'.  Verify
  // the token after the star is a ']'.  Since stars in arrays are
  // infrequent, use of lookahead is not costly here.
  if (Tok.is(tok::star) && GetLookAheadToken(1).is(tok::r_square)) {
    ConsumeToken();  // Eat the '*'.

    if (StaticLoc.isValid()) {
      Diag(StaticLoc, diag::err_unspecified_vla_size_with_static);
      StaticLoc = SourceLocation();  // Drop the static.
    }
    isStar = true;
  } else if (Tok.isNot(tok::r_square)) {
    // Note, in C89, this production uses the constant-expr production instead
    // of assignment-expr.  The only difference is that assignment-expr allows
    // things like '=' and '*='.  Sema rejects these in C89 mode because they
    // are not i-c-e's, so we don't need to distinguish between the two here.

    // Parse the constant-expression or assignment-expression now (depending
    // on dialect).
    if (getLangOpts().CPlusPlus) {
      NumElements = ParseConstantExpression();
    } else {
      EnterExpressionEvaluationContext Unevaluated(Actions,
                                                   Sema::ConstantEvaluated);
      NumElements = ParseAssignmentExpression();
    }
  }

  // If there was an error parsing the assignment-expression, recover.
  if (NumElements.isInvalid()) {
    D.setInvalidType(true);
    // If the expression was invalid, skip it.
    SkipUntil(tok::r_square, StopAtSemi);
    return;
  }

  T.consumeClose();

  ParsedAttributes attrs(AttrFactory);
  MaybeParseCXX11Attributes(attrs);

  // Remember that we parsed a array type, and remember its features.
  D.AddTypeInfo(DeclaratorChunk::getArray(DS.getTypeQualifiers(),
                                          StaticLoc.isValid(), isStar,
                                          NumElements.release(),
                                          T.getOpenLocation(),
                                          T.getCloseLocation()),
                attrs, T.getCloseLocation());
}

/// [GNU]   typeof-specifier:
///           typeof ( expressions )
///           typeof ( type-name )
/// [GNU/C++] typeof unary-expression
///
void Parser::ParseTypeofSpecifier(DeclSpec &DS) {
  assert(Tok.is(tok::kw_typeof) && "Not a typeof specifier");
  Token OpTok = Tok;
  SourceLocation StartLoc = ConsumeToken();

  const bool hasParens = Tok.is(tok::l_paren);

  EnterExpressionEvaluationContext Unevaluated(Actions, Sema::Unevaluated,
                                               Sema::ReuseLambdaContextDecl);

  bool isCastExpr;
  ParsedType CastTy;
  SourceRange CastRange;
  ExprResult Operand = ParseExprAfterUnaryExprOrTypeTrait(OpTok, isCastExpr,
                                                          CastTy, CastRange);
  if (hasParens)
    DS.setTypeofParensRange(CastRange);

  if (CastRange.getEnd().isInvalid())
    // FIXME: Not accurate, the range gets one token more than it should.
    DS.SetRangeEnd(Tok.getLocation());
  else
    DS.SetRangeEnd(CastRange.getEnd());

  if (isCastExpr) {
    if (!CastTy) {
      DS.SetTypeSpecError();
      return;
    }

    const char *PrevSpec = 0;
    unsigned DiagID;
    // Check for duplicate type specifiers (e.g. "int typeof(int)").
    if (DS.SetTypeSpecType(DeclSpec::TST_typeofType, StartLoc, PrevSpec,
                           DiagID, CastTy,
                           Actions.getASTContext().getPrintingPolicy()))
      Diag(StartLoc, DiagID) << PrevSpec;
    return;
  }

  // If we get here, the operand to the typeof was an expresion.
  if (Operand.isInvalid()) {
    DS.SetTypeSpecError();
    return;
  }

  // We might need to transform the operand if it is potentially evaluated.
  Operand = Actions.HandleExprEvaluationContextForTypeof(Operand.get());
  if (Operand.isInvalid()) {
    DS.SetTypeSpecError();
    return;
  }

  const char *PrevSpec = 0;
  unsigned DiagID;
  // Check for duplicate type specifiers (e.g. "int typeof(int)").
  if (DS.SetTypeSpecType(DeclSpec::TST_typeofExpr, StartLoc, PrevSpec,
                         DiagID, Operand.get(),
                         Actions.getASTContext().getPrintingPolicy()))
    Diag(StartLoc, DiagID) << PrevSpec;
}

/// [C11]   atomic-specifier:
///           _Atomic ( type-name )
///
void Parser::ParseAtomicSpecifier(DeclSpec &DS) {
  assert(Tok.is(tok::kw__Atomic) && NextToken().is(tok::l_paren) &&
         "Not an atomic specifier");

  SourceLocation StartLoc = ConsumeToken();
  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.consumeOpen())
    return;

  TypeResult Result = ParseTypeName();
  if (Result.isInvalid()) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return;
  }

  // Match the ')'
  T.consumeClose();

  if (T.getCloseLocation().isInvalid())
    return;

  DS.setTypeofParensRange(T.getRange());
  DS.SetRangeEnd(T.getCloseLocation());

  const char *PrevSpec = 0;
  unsigned DiagID;
  if (DS.SetTypeSpecType(DeclSpec::TST_atomic, StartLoc, PrevSpec,
                         DiagID, Result.release(),
                         Actions.getASTContext().getPrintingPolicy()))
    Diag(StartLoc, DiagID) << PrevSpec;
}


/// TryAltiVecVectorTokenOutOfLine - Out of line body that should only be called
/// from TryAltiVecVectorToken.
bool Parser::TryAltiVecVectorTokenOutOfLine() {
  Token Next = NextToken();
  switch (Next.getKind()) {
  default: return false;
  case tok::kw_short:
  case tok::kw_long:
  case tok::kw_signed:
  case tok::kw_unsigned:
  case tok::kw_void:
  case tok::kw_char:
  case tok::kw_int:
  case tok::kw_float:
  case tok::kw_double:
  case tok::kw_bool:
  case tok::kw___pixel:
    Tok.setKind(tok::kw___vector);
    return true;
  case tok::identifier:
    if (Next.getIdentifierInfo() == Ident_pixel) {
      Tok.setKind(tok::kw___vector);
      return true;
    }
    if (Next.getIdentifierInfo() == Ident_bool) {
      Tok.setKind(tok::kw___vector);
      return true;
    }
    return false;
  }
}

bool Parser::TryAltiVecTokenOutOfLine(DeclSpec &DS, SourceLocation Loc,
                                      const char *&PrevSpec, unsigned &DiagID,
                                      bool &isInvalid) {
  const PrintingPolicy &Policy = Actions.getASTContext().getPrintingPolicy();
  if (Tok.getIdentifierInfo() == Ident_vector) {
    Token Next = NextToken();
    switch (Next.getKind()) {
    case tok::kw_short:
    case tok::kw_long:
    case tok::kw_signed:
    case tok::kw_unsigned:
    case tok::kw_void:
    case tok::kw_char:
    case tok::kw_int:
    case tok::kw_float:
    case tok::kw_double:
    case tok::kw_bool:
    case tok::kw___pixel:
      isInvalid = DS.SetTypeAltiVecVector(true, Loc, PrevSpec, DiagID, Policy);
      return true;
    case tok::identifier:
      if (Next.getIdentifierInfo() == Ident_pixel) {
        isInvalid = DS.SetTypeAltiVecVector(true, Loc, PrevSpec, DiagID,Policy);
        return true;
      }
      if (Next.getIdentifierInfo() == Ident_bool) {
        isInvalid = DS.SetTypeAltiVecVector(true, Loc, PrevSpec, DiagID,Policy);
        return true;
      }
      break;
    default:
      break;
    }
  } else if ((Tok.getIdentifierInfo() == Ident_pixel) &&
             DS.isTypeAltiVecVector()) {
    isInvalid = DS.SetTypeAltiVecPixel(true, Loc, PrevSpec, DiagID, Policy);
    return true;
  } else if ((Tok.getIdentifierInfo() == Ident_bool) &&
             DS.isTypeAltiVecVector()) {
    isInvalid = DS.SetTypeAltiVecBool(true, Loc, PrevSpec, DiagID, Policy);
    return true;
  }
  return false;
}
